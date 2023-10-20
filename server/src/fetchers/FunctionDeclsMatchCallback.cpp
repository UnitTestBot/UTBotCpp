#include "FunctionDeclsMatchCallback.h"

#include "Fetcher.h"
#include "NameDecorator.h"
#include "clang-utils/ASTPrinter.h"
#include "clang-utils/AlignmentFetcher.h"
#include "clang-utils/ClangUtils.h"
#include "utils/LogUtils.h"

#include "loguru.h"

using namespace clang;

FunctionDeclsMatchCallback::FunctionDeclsMatchCallback(const Fetcher *parent,
                                                       bool onlyNames,
                                                       bool toResolveReturnTypes,
                                                       bool onlyReturnTypes)
        : parent(parent), typesResolver(parent), onlyNames(onlyNames),
          toResolveReturnTypes(toResolveReturnTypes), onlyReturnTypes(onlyReturnTypes) {
}

void FunctionDeclsMatchCallback::run(const MatchFinder::MatchResult &Result) {
    ExecUtils::throwIfCancelled();
    if (const FunctionDecl *FS = ClangUtils::getFunctionOrConstructor(Result)) {
        if (FS->isTemplated()) {
            return;
        }
        ExecUtils::throwIfCancelled();
        SourceManager &sourceManager = Result.Context->getSourceManager();
        fs::path sourceFilePath = ClangUtils::getSourceFilePath(sourceManager);

        Tests::MethodDescription methodDescription;
        if (const CXXConstructorDecl *CS = ClangUtils::getConstructor(Result)) {
            methodDescription.constructorInfo = CS->isMoveConstructor() ? Tests::ConstructorInfo::MOVE_CONSTRUCTOR
                                                                        : Tests::ConstructorInfo::CONSTRUCTOR;
        }

        std::string methodName = FS->getQualifiedNameAsString();
        std::string enumReturnTypeName = PrinterUtils::getEnumReturnMangledTypeName(methodName);
        methodDescription.name = methodName;
        methodDescription.callName = ClangUtils::getCallName(FS);
        methodDescription.sourceFilePath = sourceFilePath;
        if (onlyNames) {
            addMethod(sourceFilePath, methodDescription);
            return;
        }
        clang::QualType realReturnType = ClangUtils::getReturnType(FS, Result);
        methodDescription.returnType = ParamsHandler::getType(realReturnType, realReturnType, sourceManager);
        methodDescription.returnType.replaceTypeNameIfUnnamed(enumReturnTypeName);
        methodDescription.accessSpecifier = types::AS_pubic;
        if (onlyReturnTypes) {
            addMethod(sourceFilePath, methodDescription);
            return;
        }
        if (toResolveReturnTypes) {
            typesResolver.resolve(realReturnType);
        }
        // we find declaration of the function from header
        const auto *FSFromHeader = FS->getFirstDecl();
        auto functionDeclHeaderLocation =
                sourceManager.getExpansionLoc(FSFromHeader->getLocation());
        auto path = fs::path(sourceManager.getFilename(functionDeclHeaderLocation).str());
        bool isHeader = Paths::isHeaderFile(path);
        if (!isHeader) {
            LOG_S(DEBUG) << "Didn't find any header with declaration of function " << methodName;
        }

        auto *nodeParent = (CXXRecordDecl *) FS->getParent();

        if (FS->isCXXClassMember() && !methodDescription.isConstructor()) {
            std::string className = nodeParent->getNameAsString();
            const clang::QualType clangClassType = nodeParent->getTypeForDecl()->getCanonicalTypeInternal();
            auto classType = ParamsHandler::getType(clangClassType, clangClassType, sourceManager);
            methodDescription.classObj = {classType,
                                          classType.typeName() + "_obj",
                                          std::nullopt};
            methodDescription.accessSpecifier = getAcessSpecifier(FS);
        }
        methodDescription.returnType = ParamsHandler::getType(realReturnType, realReturnType, sourceManager);
        methodDescription.returnType.replaceTypeNameIfUnnamed(enumReturnTypeName);
        methodDescription.hasIncompleteReturnType = ClangUtils::isIncomplete(realReturnType);
        if (toResolveReturnTypes) {
            typesResolver.resolve(realReturnType);
        }
        auto returnVarName =
                NameDecorator::decorate(PrinterUtils::getReturnMangledName(methodName));
        const QualType pType = realReturnType->getPointeeType();
        if (pType.getTypePtrOrNull()) {
            addFunctionPointer(methodDescription.functionPointers,
                               pType->getAs<clang::FunctionType>(), realReturnType,
                               returnVarName, sourceManager, methodDescription.returnType);
        }
        methodDescription.modifiers.isStatic = FS->isStatic();
        methodDescription.modifiers.isExtern = FS->getStorageClass() == SC_Extern;
        methodDescription.modifiers.isInline = FS->isInlined();
        methodDescription.isVariadic = FS->isVariadic();
        methodDescription.paramsString =
                ASTPrinter::getSourceText(FS->getParametersSourceRange(), sourceManager);
        if (FS->hasBody() && parent->fetchFunctionBodies) {
            methodDescription.sourceBody =
                    ASTPrinter::getSourceText(FS->getBody()->getSourceRange(), sourceManager);
        }


        const auto paramsFromDefinition = FS->parameters();
        const auto paramsFromDeclaration = FSFromHeader->parameters();
        for (size_t i = 0; i < paramsFromDeclaration.size(); ++i) {
            const auto &declParam = paramsFromDeclaration[i];
            const auto &defParam = paramsFromDefinition[i];
            std::string name = NameDecorator::decorate(defParam->getNameAsString());
            std::string mangledName = PrinterUtils::getParamMangledName(name, methodName);
            if (name.empty()) {
                return;
            }
            if (name == methodDescription.name) {
                name = mangledName;
            }
            auto paramType = ParamsHandler::getType(defParam->getType(), declParam->getType(), sourceManager);
            addFunctionPointer(methodDescription.functionPointers, declParam->getFunctionType(),
                               declParam->getType(), name, sourceManager, paramType);
            auto alignment = AlignmentFetcher::fetch(defParam);
            bool hasIncompleteType = ClangUtils::isIncomplete(defParam->getType());
            methodDescription.params.emplace_back(paramType, name, alignment, hasIncompleteType);
        }
        if (CollectionUtils::contains(methods[sourceFilePath], methodDescription)) {
            LOG_S(ERROR) << "Method " << methodDescription.name << " from " << sourceFilePath
                         << " was matched by clang the second time for some reason, skipping "
                            "method duplicate.";
            return;
        }
        addMethod(sourceFilePath, methodDescription);
        logFunction(methodDescription, sourceFilePath);
    }
}

void FunctionDeclsMatchCallback::addMethod(const fs::path &sourceFilePath,
                                           const Tests::MethodDescription &methodDescription) {
    Tests &tests = (*parent->projectTests).at(sourceFilePath);
    tests.methods.emplace(methodDescription.name, methodDescription);
    methods[sourceFilePath].insert(methodDescription);
}

void FunctionDeclsMatchCallback::logFunction(const Tests::MethodDescription &description,
                                             const fs::path &path) {
    std::stringstream logStream;
    logStream << "Fetched function: " << description.name;
    if (LogUtils::isMaxVerbosity()) {
        logStream << "\n\tLocation: " << path;
        logStream << "\n\tDeclaration: " << description.returnType.usedType() << " ";
    }
    if (LogUtils::isMaxVerbosity()) {
        logStream << "(";
        for (int i = 0; i < description.params.size(); i++) {
            logStream << description.params[i].type.usedType() << " " << description.params[i].name;
            if (i != description.params.size() - 1) {
                logStream << ", ";
            }
        }
        logStream << ");";
    }
    LOG_S(DEBUG) << logStream.str();
}

void FunctionDeclsMatchCallback::addFunctionPointer(
        tests::Tests::MethodDescription::FPointerMap &functionPointers,
        const clang::FunctionType *functionType,
        const clang::QualType &qualType,
        const std::string &name,
        const clang::SourceManager &sourceManager,
        const types::Type &type) {
    if (type.isPointerToFunction()) {
        if (functionType) {
            functionPointers[name] = ParamsHandler::getFunctionPointerDeclaration(functionType, name, sourceManager,
                                                                                  false);
        } else {
            LOG_S(WARNING) << "Type '" << name << "' fetch as function pointer but can't get functionType";
        }
    } else if (type.isArrayOfPointersToFunction()) {
        const clang::FunctionType *functionType = qualType->getPointeeType()->getPointeeType()->getAs<clang::FunctionType>();
        if (functionType) {
            functionPointers[name] = ParamsHandler::getFunctionPointerDeclaration(
                    functionType, name,
                    sourceManager, true);
        } else {
            LOG_S(WARNING) << "Type '" << name << "' fetch as function pointer but can't get functionType";
        }
    }
}
