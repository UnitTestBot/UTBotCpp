#include "SourceToHeaderMatchCallback.h"

#include "Matchers.h"
#include "NameDecorator.h"
#include "SourceToHeaderRewriter.h"
#include "printers/Printer.h"
#include "utils/ExecUtils.h"
#include "clang-utils/ClangUtils.h"

#include "loguru.h"

#include <utility>

using namespace clang;
using namespace Matchers;
using StringUtils::stringFormat;

SourceToHeaderMatchCallback::SourceToHeaderMatchCallback(utbot::ProjectContext projectContext,
                                                         fs::path sourceFilePath,
                                                         raw_ostream *externalStream,
                                                         raw_ostream *internalStream,
                                                         raw_ostream *wrapperStream,
                                                         bool forStubHeader)
    : projectContext(std::move(projectContext)),
      sourceFilePath(std::move(sourceFilePath)), externalStream(externalStream),
      internalStream(internalStream), wrapperStream(wrapperStream), forStubHeader(forStubHeader) {
}

void SourceToHeaderMatchCallback::run(const ast_matchers::MatchFinder::MatchResult &Result) {
    ExecUtils::throwIfCancelled();
    checkStruct(Result);
    checkEnum(Result);
    checkUnion(Result);
    checkTypedef(Result);
    checkFunctionDecl(Result);
    checkVarDecl(Result);
}

void SourceToHeaderMatchCallback::checkStruct(const MatchFinder::MatchResult &Result) {
    if (const auto *ST = Result.Nodes.getNodeAs<RecordDecl>(TOPLEVEL_STRUCT_OR_CLASS_DECL)) {
        std::string name = ST->getNameAsString();
        if (!name.empty()) {
            handleStruct(ST);
        }
    }
}

void SourceToHeaderMatchCallback::checkEnum(const MatchFinder::MatchResult &Result) {
    if (const auto *EN = Result.Nodes.getNodeAs<EnumDecl>(TOPLEVEL_ENUM_DECL)) {
        std::string name = EN->getNameAsString();
        if (!name.empty()) {
            handleEnum(EN);
        }
    }
}

void SourceToHeaderMatchCallback::checkUnion(const MatchFinder::MatchResult &Result) {
    if (const auto *decl = Result.Nodes.getNodeAs<RecordDecl>(INNER_TYPEDEF_UNION_DECL)) {
        std::string name = decl->getNameAsString();
        if (name.empty()) {
            if (const auto *TD = Result.Nodes.getNodeAs<TypedefDecl>(TYPEDEF_UNION_DECL)) {
                handleUnion(decl);
            }
        }
    }
    if (const auto *decl = Result.Nodes.getNodeAs<RecordDecl>(TOPLEVEL_UNION_DECL)) {
        std::string name = decl->getNameAsString();
        if (!name.empty()) {
            handleUnion(decl);
        }
    }
}

void SourceToHeaderMatchCallback::checkTypedef(const MatchFinder::MatchResult &Result) {
    if (const auto *decl = Result.Nodes.getNodeAs<TypedefDecl>(TOPLEVEL_TYPEDEF)) {
        if (!decl->isImplicit()) {
            handleTypedef(decl);
        }
    }
}

void SourceToHeaderMatchCallback::checkFunctionDecl(
    const ast_matchers::MatchFinder::MatchResult &Result) {
    if (const FunctionDecl *decl = ClangUtils::getFunctionOrConstructor(Result)) {
        if (decl->isInlined() && decl->getStorageClass() == SC_None) {
            LOG_S(DEBUG)
                << "inline function without static or extern modifier is not supported by now";
            /*
             * No-optimization build is entirely different. Compilers usually do not perform
             * inlining in this mode. Instead, they treat inline function definitions as
             * declarations only. In rare circumstances this happens even with -O3. For example,
             * this happens when function recursively calls itself. Or when a function is too big
             * but is referenced multiple times: compiler concludes that it is cheaper to call
             * separate function instead of inlining it in all places. Inline keyword is a mere
             * hint. If you need guaranteed inlining, you have to use compiler-dependent extensions,
             * such as __attribute__((always_inline)) in gcc.
             */
            return;
        }
        if (decl->hasBody()) {
            handleFunctionDecl(decl);
        }
    }
}

void SourceToHeaderMatchCallback::checkVarDecl(const MatchFinder::MatchResult &Result) {
    if (const auto *decl = Result.Nodes.getNodeAs<VarDecl>(TOPLEVEL_VAR_DECL)) {
        auto name = decl->getNameAsString();
        if (decl->getType()->hasUnnamedOrLocalType()) {
            LOG_S(DEBUG) << "Variable declaration \'" << name
                         << "\' skipped as: hasUnnamedOrLocalType";
            return;
        }
        if (decl->isExternallyDeclarable() && !decl->isKnownToBeDefined()) {
            LOG_S(DEBUG) << "Variable \"" << name << "\" was skipped - it has no definition.";
            return;
        }
        auto [iterator, inserted] = variables.insert(name);
        if (inserted) {
            handleVarDecl(decl);
        } else {
            LOG_S(DEBUG) << "Variable declaration \'" << name
                         << "\' was matched by clang the second time. Probably variable has extern "
                            "modifier.";
        }
    }
}

void SourceToHeaderMatchCallback::handleStruct(const RecordDecl *decl) {
    print(decl);
}
void SourceToHeaderMatchCallback::handleEnum(const EnumDecl *decl) {
    print(decl);
}
void SourceToHeaderMatchCallback::handleUnion(const RecordDecl *decl) {
    print(decl);
}

void SourceToHeaderMatchCallback::handleTypedef(const TypedefDecl *decl) {
    auto policy = getDefaultPrintingPolicy(decl, true);
    auto canonicalType = decl->getUnderlyingType().getCanonicalType();
    auto name = decl->getName().str();
    auto canonicalName = canonicalType.getAsString();
    if (name == "wchar_t" && !forStubHeader) {
        // wchar_t is builtin type in C++ but is typedef in C, so let's define it
        if (externalStream != nullptr) {
            *externalStream << NameDecorator::defineWcharT(canonicalName) << "\n";
        }
        return;
    }
    if (name == canonicalName) {
        /*
         * Without that policy declaration for
         * @code typedef struct { int x; } Y;
         * would be printed as
         * @code typedef struct Y Y;
         * instead of
         * @code typedef struct { int x; } Y;
         */
        policy.IncludeTagDefinition = 1;
    }
    print(decl, policy);
}

void SourceToHeaderMatchCallback::handleFunctionDecl(const FunctionDecl *decl) {
    // remove static modifier
    auto storageClass = decl->getStorageClass();
    bool isInlineSpecified = decl->isInlineSpecified();
    auto *mutableDecl = const_cast<FunctionDecl *>(decl);
    mutableDecl->setStorageClass(SC_None);
    // remove inline modifier
    mutableDecl->setInlineSpecified(false);

    generateInternal(decl);
    generateWrapper(decl);

    mutableDecl->setInlineSpecified(isInlineSpecified);
    mutableDecl->setStorageClass(storageClass);
}

void SourceToHeaderMatchCallback::handleVarDecl(const VarDecl *decl) {
    StorageClass storageClass = decl->getStorageClass();
    auto *mutableDecl = const_cast<VarDecl *>(decl);
    mutableDecl->setStorageClass(SC_None);

    generateInternal(decl);
    generateWrapper(decl);

    mutableDecl->setStorageClass(storageClass);
}

void SourceToHeaderMatchCallback::generateInternal(const FunctionDecl *decl) const {
    if (internalStream == nullptr) {
        return;
    }
    auto policy = getDefaultPrintingPolicy(decl, true);
    policy.TerseOutput = 1;
    policy.PolishForDeclaration = 1;
    policy.AnonymousTagLocations = 0;

    std::string name = decl->getNameAsString();
    std::string decoratedName = decorate(name);
    std::string wrapperName = PrinterUtils::wrapperName(name, projectContext, sourceFilePath);

    std::string curDecl = getRenamedDeclarationAsString(decl, policy, decoratedName);
    std::string wrapperDecl = getRenamedDeclarationAsString(decl, policy, wrapperName);

    if (IsOldStyleDefinition(curDecl)){
        curDecl = getOldStyleDeclarationAsString(decl, decoratedName);
        wrapperDecl = getOldStyleDeclarationAsString(decl, wrapperName);
    }
    std::string enumReturnTypeName = PrinterUtils::getEnumReturnTypeName(name);
    replaceAnonymousEnumTypeName(wrapperDecl, enumReturnTypeName);
    replaceAnonymousEnumTypeName(curDecl, enumReturnTypeName);

    auto enumDecl = llvm::dyn_cast_or_null<clang::EnumDecl>(decl->getReturnType()->getAsTagDecl());
    if (enumDecl && !enumDecl->hasNameForLinkage()) {
        std::string enumDeclTypeName = PrinterUtils::getEnumDeclTypeName(name);
        renameDecl(enumDecl, enumDeclTypeName);
        print(enumDecl);
    }

    *internalStream << "extern \"C\" " << wrapperDecl << ";\n";
    *internalStream << "static " << curDecl << " {\n";
    printReturn(decl, wrapperName, internalStream);
    *internalStream << "}\n";
}

void SourceToHeaderMatchCallback::generateInternal(const VarDecl *decl) const {
    if (internalStream == nullptr) {
        return;
    }
    auto policy = getDefaultPrintingPolicy(decl, true);
    policy.SuppressInitializers = 1;

    /*
     * extern "C" (*var_wrapper);
     * &DECL = *var_wrapper;
     */

    std::string name = decl->getNameAsString();
    std::string decoratedName = decorate(name);
    std::string wrapperName = PrinterUtils::wrapperName(name, projectContext, sourceFilePath);
    std::string wrapperPointerName = stringFormat("(*%s)", wrapperName);
    std::string refName = stringFormat("(&%s)", decoratedName);

    std::string curDecl;
    llvm::raw_string_ostream curDeclStream{ curDecl };
    decl->print(curDeclStream, policy);
    std::string wrapperPointerDecl =
        getRenamedDeclarationAsString(decl, policy, wrapperPointerName);
    std::string refDecl = getRenamedDeclarationAsString(decl, policy, refName);

    *internalStream << "extern \"C\" " << wrapperPointerDecl << ";\n";
    *internalStream << refDecl << " = " << wrapperPointerName << ";\n";
}

void SourceToHeaderMatchCallback::generateWrapper(const FunctionDecl *decl) const {
    if (wrapperStream == nullptr) {
        return;
    }
    auto policy = getDefaultPrintingPolicy(decl, false);
    policy.TerseOutput = 1;
    policy.PolishForDeclaration = 1;
    policy.AnonymousTagLocations = 0;

    /*
     * fun_wrapper {
     * return fun(args...);
     * }
     */
    std::string name = decl->getNameAsString();
    std::string wrapperName = PrinterUtils::wrapperName(name, projectContext, sourceFilePath);
    std::string wrapperDecl = getRenamedDeclarationAsString(decl, policy, wrapperName);
    if (IsOldStyleDefinition(wrapperDecl)){
        wrapperDecl = getOldStyleDeclarationAsString(decl, wrapperName);
    }
    replaceAnonymousEnumTypeName(wrapperDecl, "int");

    *wrapperStream << wrapperDecl << " {\n";
    printReturn(decl, name, wrapperStream);
    *wrapperStream << "}\n";
}

void SourceToHeaderMatchCallback::generateWrapper(const VarDecl *decl) const {
    if (wrapperStream == nullptr) {
        return;
    }
    auto policy = getDefaultPrintingPolicy(decl, false);
    policy.SuppressInitializers = 1;

    /*
     * (*var_wrapper) = &var;
     */

    std::string name = decl->getNameAsString();
    std::string wrapperName = PrinterUtils::wrapperName(name, projectContext, sourceFilePath);
    std::string wrapperPointerName = stringFormat("(*%s)", wrapperName);
    std::string wrapperPointerDecl =
        getRenamedDeclarationAsString(decl, policy, wrapperPointerName);
    *wrapperStream << wrapperPointerDecl << " = &" << name << ";\n";
}


void SourceToHeaderMatchCallback::printReturn(const FunctionDecl *decl,
                                              std::string const &name,
                                              raw_ostream *stream) const {
    if (stream == nullptr) {
        return;
    }
    printer::Printer printer;
    auto args = CollectionUtils::transformTo<std::vector<std::string>>(
        decl->parameters(), [](ParmVarDecl *param) { return param->getNameAsString(); });
    bool noReturn = decl->isNoReturn() || decl->getReturnType()->isVoidType();
    if (!noReturn) {
        printer.ss << "return ";
    }
    printer.strFunctionCall(name, args);

    *stream << printer.ss.str();
}

std::string SourceToHeaderMatchCallback::decorate(std::string_view name) const {
    return forStubHeader ? std::string(name) : NameDecorator::decorate(name);
}


void SourceToHeaderMatchCallback::print(const NamedDecl *decl, const PrintingPolicy &policy) const {
    if (externalStream == nullptr) {
        return;
    }
    auto pAlignmentAttr = decl->getAttr<MaxFieldAlignmentAttr>();
    if (pAlignmentAttr) {
        unsigned int alignment = pAlignmentAttr->getAlignment() / 8;
        *externalStream << stringFormat("#pragma pack(push, %d)\n", alignment);
    }
    auto name = decl->getNameAsString();
    auto decoratedName = decorate(name);
    auto declaration = getRenamedDeclarationAsString(decl, policy, decoratedName);
    *externalStream << declaration << ";\n";
    if (pAlignmentAttr) {
        *externalStream << "#pragma pack(pop)\n";
    }
    *externalStream << "\n";
}

void SourceToHeaderMatchCallback::print(const NamedDecl *decl) const {
    print(decl, getDefaultPrintingPolicy(decl, true));
}

PrintingPolicy
SourceToHeaderMatchCallback::getDefaultPrintingPolicy(const Decl *decl,
                                                      bool adjustForCPlusPlus) const {
    clang::ASTContext const &context = decl->getASTContext();
    clang::LangOptions const &langOptions = context.getLangOpts();
    PrintingPolicy policy{ langOptions };
    if (forStubHeader) {
        return policy;
    }
    if (adjustForCPlusPlus) {
        policy.adjustForCPlusPlus();
        policy.Restrict = 0;
        policy.Alignof = 1;
        policy.UnderscoreAlignof = 0;
    }
    return policy;
}

std::string
SourceToHeaderMatchCallback::getRenamedDeclarationAsString(const clang::NamedDecl *decl,
                                                           clang::PrintingPolicy const &policy,
                                                           std::string const &name) const {
    std::string result;
    llvm::raw_string_ostream resultStream{ result };
    auto curDeclName = decl->getDeclName();
    renameDecl(decl, name);
    decl->print(resultStream, policy);
    const_cast<NamedDecl *>(decl)->setDeclName(curDeclName);
    resultStream.flush();
    return result;
}

void SourceToHeaderMatchCallback::renameDecl(const NamedDecl *decl, const std::string &name) const {
    auto &info = decl->getASTContext().Idents.get(name);
    DeclarationName wrapperDeclarationName{ &info };
    const_cast<NamedDecl *>(decl)->setDeclName(wrapperDeclarationName);
}

void SourceToHeaderMatchCallback::replaceAnonymousEnumTypeName(std::string& decl, const std::string& typeName) {
    StringUtils::replaceFirst(decl, "enum (anonymous)", typeName);
}

std::string SourceToHeaderMatchCallback::getOldStyleDeclarationAsString(const FunctionDecl *decl, std::string const &name) const{
    std::string result;
    llvm::raw_string_ostream resultStream{ result };
    std::string funcReturnType = decl->getFunctionType()->getReturnType().getAsString();
    std::vector<std::string> parameters = CollectionUtils::transformTo<std::vector<std::string>>(
        decl->parameters(), [](ParmVarDecl *param) { return param->getNameAsString(); });

    std::vector<std::string> param_types = CollectionUtils::transformTo<std::vector<std::string>>(
        decl->parameters(), [](ParmVarDecl *param) { return param->getType().getAsString(); });
    resultStream << funcReturnType << ' ' << name << '(';

    for (int i = 0; i < parameters.size(); i++){
        if (i != 0){
            resultStream << ", ";
        }
        resultStream << param_types[i] << ' ' << parameters[i];
    }
    resultStream << ")";
    resultStream.flush();
    return result;
}

/*Example of old style definition
int sum(a, b)
int a;
int b;
{
    return a + b;
}
*/
bool SourceToHeaderMatchCallback::IsOldStyleDefinition(std::string const &definition) const{
    std::regex normStyle ("\\([a-zA-Z0-9*&_()\\[\\]]+ [a-zA-Z0-9*&_()\\[\\]]+[,) ]");
    bool res = std::regex_search(definition, normStyle);
    return !res;
}
