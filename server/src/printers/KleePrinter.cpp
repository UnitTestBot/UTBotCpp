/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "KleePrinter.h"

#include "KleeConstraintsPrinter.h"
#include "PathSubstitution.h"
#include "Paths.h"
#include "exceptions/NoSuchTypeException.h"
#include "exceptions/UnImplementedException.h"
#include "utils/CollectionUtils.h"
#include "utils/FileSystemUtils.h"
#include "utils/KleeUtils.h"
#include "visitors/KleeAssumeParamVisitor.h"
#include "visitors/KleeAssumeReturnValueVisitor.h"

#include "loguru.h"

#include <unordered_set>
#include <fstream>

using namespace types;
using printer::KleePrinter;

static const std::string KLEE_GLOBAL_VAR_H = "klee_global_var.h";

printer::KleePrinter::KleePrinter(const types::TypesHandler *typesHandler,
                                  std::shared_ptr<BuildDatabase> buildDatabase,
                                  utbot::Language srcLanguage)
    : Printer(srcLanguage), typesHandler(typesHandler), buildDatabase(std::move(buildDatabase)) {
}

fs::path KleePrinter::writeTmpKleeFile(
    const Tests &tests,
    const string &buildDir,
    const PathSubstitution &pathSubstitution,
    const std::optional<LineInfo::PredicateInfo> &predicateInfo,
    const string &testedMethod,
    const std::optional<string> &testedClass,
    bool onlyForOneFunction,
    bool onlyForOneClass,
    bool isConstructor,
    const std::function<bool(tests::Tests::MethodDescription const &)> &methodFilter) {

    resetStream();
    writeCopyrightHeader();

    bool onlyForOneEntity = onlyForOneFunction || onlyForOneClass;
    auto unitInfo = buildDatabase->getClientCompilationUnitInfo(tests.sourceFilePath);
    string kleeFilePath = unitInfo->kleeFilesInfo->getKleeFile(testedMethod);

    LOG_S(DEBUG) << "Writing tmpKleeFile for " << testedMethod << " inside " << tests.sourceFilePath;

    bool hasMethod = false;
    for (const auto &[methodName,testMethod ]: tests.methods) {
        if (methodFilter(testMethod)) {
            hasMethod = true;
        }
    }

    auto headers = getIncludePaths(tests, pathSubstitution);
    for (const auto &header : headers) {
        LOG_S(MAX) << "Header is included in tmpKleeFile: " << header;
        strInclude(header);
    }

    if (!hasMethod) {
        FileSystemUtils::writeToFile(kleeFilePath, ss.str());
        LOG_S(DEBUG) << "TmpKleeFile written to " << kleeFilePath;
        return kleeFilePath;
    }

    bool predicate = predicateInfo.has_value();
    if (!onlyForOneEntity && !testedMethod.empty() && !predicate) {
        strInclude(KLEE_GLOBAL_VAR_H);
        strDeclareVar("int", PrinterUtils::KLEE_PATH_FLAG, "0");
    }

    strInclude("klee/klee.h");
    strInclude("stdlib.h", true);
    ss << NL;
    writeStubsForStructureFields(tests);

    writeAccessPrivateMacros(typesHandler, tests, false);

    for (const auto &[methodName, testMethod] : tests.methods) {
        if (!methodFilter(testMethod)) {
            continue;
        }
        if ((onlyForOneFunction && methodName != testedMethod) ||
            (onlyForOneClass && testMethod.isClassMethod() && testMethod.classObj->type.typeName() != testedClass)) {
            continue;
        }
        try {
            writeStubsForFunctionParams(typesHandler, testMethod, true);
            declTestEntryPoint(tests, testMethod);
            genGlobalParamsDeclarations(testMethod);
            genParamsDeclarations(testMethod);
            genPostGlobalSymbolicVariables(testMethod);
            genPostParamsSymbolicVariables(testMethod);
            if (types::TypesHandler::skipTypeInReturn(testMethod.returnType)) {
                genVoidFunctionAssumes(testMethod, predicateInfo, testedMethod, onlyForOneEntity);
            } else {
                genNonVoidFunctionAssumes(testMethod, predicateInfo, testedMethod,
                                          onlyForOneEntity, isConstructor);
            }
            genGlobalsKleeAssumes(testMethod);
            genPostParamsKleeAssumes(testMethod);
            strReturn("0");
            closeBrackets(1);
        } catch (const UnImplementedException &e) {
            std::string message =
                "Could not generate klee code for method \'" + methodName + "\', skipping it. ";
            LOG_S(WARNING) << message << e.what();
        }
    }

    FileSystemUtils::writeToFile(kleeFilePath, ss.str());
    LOG_S(DEBUG) << "TmpKleeFile written to " << kleeFilePath;
    return kleeFilePath;
}

void KleePrinter::declTestEntryPoint(const Tests &tests,
                                     const Tests::MethodDescription &testMethod) {
    string entryPoint = KleeUtils::entryPointFunction(tests, testMethod.name);
    auto argvType = types::Type::createSimpleTypeFromName("char", 2);
    // if change args in next line also change cpp mangledPath in kleeUtils.cpp
    strFunctionDecl("int", entryPoint, {types::Type::intType(), argvType, argvType}, {"utbot_argc", "utbot_argv", "utbot_envp"}, "") << LB();
}

Tests::MethodParam KleePrinter::getKleeMethodParam(tests::Tests::MethodParam const &param) {
    if (param.type.isTwoDimensionalPointer()) {
        return { param.type, param.underscoredName(), param.alignment };
    } else {
        return param;
    }
}

Tests::MethodParam KleePrinter::getKleePostParam(const Tests::MethodParam &param) {
    auto postVariable = KleeUtils::postSymbolicVariable(param.name);
    return { param.type, postVariable, param.alignment };
}

Tests::MethodParam KleePrinter::getKleeGlobalParam(tests::Tests::MethodParam const &param) {
    if (param.type.isObjectPointer()) {
        return { param.type, param.underscoredName(), param.alignment };
    } else {
        return param;
    }
}

Tests::MethodParam KleePrinter::getKleeGlobalPostParam(const Tests::MethodParam &globalParam) {
    auto postVariable = KleeUtils::postSymbolicVariable(globalParam.name);
    if (globalParam.type.isObjectPointer()) {
        return { globalParam.type.baseTypeObj(), postVariable, globalParam.alignment };
    } else {
        return { globalParam.type, postVariable, globalParam.alignment };
    }
}

void KleePrinter::genPostGlobalSymbolicVariables(const Tests::MethodDescription &testMethod) {
    for (const auto &globalParam : testMethod.globalParams) {
        genPostSymbolicVariable(testMethod, getKleeGlobalPostParam(globalParam));
    }
}

void KleePrinter::genPostParamsSymbolicVariables(const Tests::MethodDescription &testMethod) {
    if (testMethod.isClassMethod()) {
        genPostSymbolicVariable(testMethod, getKleePostParam(testMethod.classObj.value()));
    }
    for (const auto &param : testMethod.params) {
        if (param.isChangeable()) {
            genPostSymbolicVariable(testMethod, getKleePostParam(param));
        }
    }
}

void KleePrinter::genPostSymbolicVariable(const Tests::MethodDescription &testMethod, const Tests::MethodParam &kleeParam) {
    bool isArray = genParamDeclaration(testMethod, kleeParam);
    strKleeMakeSymbolic(kleeParam.type, kleeParam.name, !isArray);
}

void KleePrinter::genGlobalsKleeAssumes(const Tests::MethodDescription &testMethod) {
    for (const auto &globalParam : testMethod.globalParams) {
        genPostAssumes(globalParam, true);
    }
}

void KleePrinter::genPostParamsKleeAssumes(const Tests::MethodDescription &testMethod) {
    if (testMethod.isClassMethod()) {
        genPostAssumes(testMethod.classObj.value());
    }
    for (auto &param : testMethod.params) {
        if (param.isChangeable()) {
            genPostAssumes(param);
        }
    }
}

void KleePrinter::genPostAssumes(const Tests::MethodParam &param, bool visitGlobal) {
    auto outVariable = KleeUtils::postSymbolicVariable(param.name);
    visitor::KleeAssumeParamVisitor paramVisitor(typesHandler, this);
    if (visitGlobal) {
        paramVisitor.visitGlobal(param, outVariable);
    } else {
        paramVisitor.visit(param, outVariable);
    }
}

string KleePrinter::addTestLineFlag(const std::shared_ptr<LineInfo> &lineInfo,
                                    bool needAssertion,
                                    const utbot::ProjectContext &projectContext) {
    resetStream();
    std::ifstream is(lineInfo->filePath);
    string currentLine;
    unsigned lineCounter = 1;
    strInclude(KLEE_GLOBAL_VAR_H);
    while (std::getline(is, currentLine)) {
        if (lineCounter == lineInfo->begin + lineInfo->insertAfter) {
            if (lineInfo->wrapInBrackets) {
                ss << BNL;
            }
            if (!needAssertion) {
                ss << PrinterUtils::KLEE_PATH_FLAG << " = 1" << SCNL;
            }
        }
        if (lineCounter == lineInfo->begin + lineInfo->insertAfter + 1) {
            if (lineInfo->wrapInBrackets) {
                ss << "}" << NL;
            }
        }
        if (lineCounter == lineInfo->begin) {
            if (needAssertion) {
                ss << "#pragma push_macro(\"assert\")\n";
                ss << "#define assert(expr) if (!(expr)) {" << PrinterUtils::KLEE_PATH_FLAG << " = 1;}" << NL;
            }
        }
        ss << currentLine << NL;
        if (lineCounter == lineInfo->begin) {
            if (needAssertion) {
                ss << "#pragma pop_macro(\"assert\")" << NL;
            }
        }
        lineCounter++;
    }
    fs::path flagFileFolder = Paths::getFlagsDir(projectContext);

    fs::path globalFlagFilePath = flagFileFolder / KLEE_GLOBAL_VAR_H;
    FileSystemUtils::writeToFile(globalFlagFilePath, StringUtils::stringFormat("extern int %s;", PrinterUtils::KLEE_PATH_FLAG));

    fs::path flagFilePath = flagFileFolder / lineInfo->filePath.filename();
    FileSystemUtils::writeToFile(flagFilePath, ss.str());
    return flagFilePath;
}

void KleePrinter::genVoidFunctionAssumes(const Tests::MethodDescription &testMethod,
                                  const std::optional<PredInfo> &predicateInfo,
                                  const string &testedMethod,
                                  bool onlyForOneEntity) {
    genKleePathSymbolicIfNeeded(predicateInfo, testedMethod, onlyForOneEntity);
    strFunctionCall(testMethod, testMethod.returnType.countReturnPointers(), SCNL, false);
    genKleePathSymbolicAssumeIfNeeded(predicateInfo, testedMethod, onlyForOneEntity);
}

void KleePrinter::genNonVoidFunctionAssumes(const Tests::MethodDescription &testMethod,
                                     const std::optional<PredInfo> &predicateInfo,
                                     const string &testedMethod,
                                     bool onlyForOneEntity, bool isConstructor) {
    genKleePathSymbolicIfNeeded(predicateInfo, testedMethod, onlyForOneEntity);
    genReturnDeclaration(testMethod, predicateInfo, isConstructor);
    genParamsKleeAssumes(testMethod, predicateInfo, testedMethod, onlyForOneEntity);
}

std::vector<std::string> KleePrinter::getIncludePaths(const Tests &tests,
                                                      const PathSubstitution &substitution) const {
    return { substitution.substituteLineFlag(tests.sourceFilePath) };
}

void KleePrinter::genGlobalParamsDeclarations(const Tests::MethodDescription &testMethod) {
    for (const auto &param : testMethod.globalParams) {
        tests::Tests::MethodParam kleeParam = getKleeGlobalParam(param);
        bool isArray = TypesHandler::isArrayType(param.type);
        if (param.type.isObjectPointer()) {
            isArray = genParamDeclaration(testMethod, kleeParam);
        }
        strKleeMakeSymbolic(kleeParam.type, kleeParam.name, param.name, !isArray);
        if (param.type.isObjectPointer()) {
            if (param.type.isTwoDimensionalPointer()) {
                genTwoDimPointers(param, false);
            } else {
                strAssignVar(param.name, kleeParam.name);
            }
        }
        genConstraints(kleeParam, testMethod.name);
    }
}

void KleePrinter::genParamsDeclarations(const Tests::MethodDescription &testMethod) {
    if (testMethod.isClassMethod()) {
        strDeclareVar(testMethod.classObj->type.typeName(), testMethod.classObj->name);
        strKleeMakeSymbolic(testMethod.classObj->type, testMethod.classObj->name, testMethod.classObj->name, true);

        KleeConstraintsPrinter constraintsPrinter(typesHandler, srcLanguage);
        constraintsPrinter.setTabsDepth(tabsDepth);
        const auto constraintsBlock = constraintsPrinter.genConstraints(testMethod.classObj->name,
                                                                        testMethod.classObj->type).str();
        ss << constraintsBlock;
    }
    for (const auto &param : testMethod.params) {
        tests::Tests::MethodParam kleeParam = getKleeMethodParam(param);
        bool isArray = genParamDeclaration(testMethod, kleeParam);
        if (CollectionUtils::containsKey(testMethod.functionPointers, param.name)) {
            continue;
        }
        auto paramType = kleeParam.type.maybeJustPointer() ? kleeParam.type.baseTypeObj() : kleeParam.type;
        strKleeMakeSymbolic(paramType, kleeParam.name, param.name, !isArray);
        genConstraints(kleeParam, testMethod.name);
        genTwoDimPointers(param, true);
        commentBlockSeparator();
    }
}

bool KleePrinter::genParamDeclaration(const Tests::MethodDescription &testMethod,
                                      const Tests::MethodParam &param) {
    string stubFunctionName =
        PrinterUtils::getFunctionPointerStubName(testMethod.isClassMethod() ? std::make_optional(testMethod.classObj->name) : std::nullopt,
                                                 testMethod.name, param.name);
    if (types::TypesHandler::isPointerToFunction(param.type)) {
        strDeclareVar(getTypedefFunctionPointer(testMethod.name, param.name, false), param.name,
                      stubFunctionName, param.alignment);
    } else if (types::TypesHandler::isArrayOfPointersToFunction(param.type)) {
        strDeclareArrayOfFunctionPointerVar(getTypedefFunctionPointer(testMethod.name, param.name, false), param.name, stubFunctionName);
    } else if (types::TypesHandler::isObjectPointerType(param.type)) {
        return genPointerParamDeclaration(param);
    } else if (types::TypesHandler::isArrayType(param.type)) {
        strDeclareArrayVar(param.type, param.name, types::PointerUsage::PARAMETER);
        return true;
    } else {
        strDeclareVar(param.type.typeName(), param.name, std::nullopt, param.alignment);
    }
    return false;
}

bool KleePrinter::genPointerParamDeclaration(const Tests::MethodParam &param) {
    std::string element = param.name;
    bool isArray = false;
    if (param.type.pointerArrayKinds().size() > 1) {
        element = constrMultiIndex(element, param.type.arraysSizes(types::PointerUsage::PARAMETER));
        isArray = true;
    } else if (!param.type.maybeJustPointer()) {
        size_t size = types::TypesHandler::getElementsNumberInPointerOneDim(
            types::PointerUsage::PARAMETER);
        element = constrIndex(element, size);
        isArray = true;
    }

    if (types::TypesHandler::isVoid(param.type.baseTypeObj())) {
        strDeclareVar(Type::minimalScalarType().baseType(), element, std::nullopt, param.alignment);
    } else {
        strDeclareVar(param.type.baseType(), element, std::nullopt, param.alignment);
    }
    return isArray;
}

void KleePrinter::makeBracketsForStrPredicate(const std::optional<PredInfo> &info) {
    if (info.has_value() && info->type == testsgen::STRING) {
        ss << "[" << info->returnValue.size() << "]";
    }
}


void KleePrinter::genReturnDeclaration(const Tests::MethodDescription &testMethod, const std::optional<PredInfo> &predicateInfo, bool isConstructor = false) {
    // If return type is a pointer, we compare values that are stored at this pointers,
    // not the pointers themselves
    Type returnType = types::TypesHandler::isVoid(testMethod.returnType.baseTypeObj())
                          ? Type::minimalScalarType()
                          : testMethod.returnType;
    bool maybeArray = returnType.maybeReturnArray();
    bool isPointer = testMethod.returnType.isObjectPointer();
    strDeclareVar(returnType.baseType(), KleeUtils::RESULT_VARIABLE_NAME, std::nullopt, std::nullopt, false);
    makeBracketsForStrPredicate(predicateInfo);
    if (maybeArray) {
        size_t size = types::TypesHandler::getElementsNumberInPointerOneDim(PointerUsage::RETURN);
        ss << "[" << size << "]";
    }
    ss << SCNL;
    strKleeMakeSymbolic(KleeUtils::RESULT_VARIABLE_NAME,
                        !maybeArray && !(predicateInfo.has_value() && predicateInfo->type == testsgen::STRING));
    if (isPointer) {
        strDeclareVar("int", KleeUtils::NOT_NULL_VARIABLE_NAME);
        strKleeMakeSymbolic(KleeUtils::NOT_NULL_VARIABLE_NAME, true);
    }
}

void KleePrinter::genParamsKleeAssumes(
    const Tests::MethodDescription &testMethod,
    const std::optional<LineInfo::PredicateInfo> &predicateInfo,
    const string &testedMethod,
    bool onlyForOneEntity) {
    visitor::KleeAssumeReturnValueVisitor(typesHandler, this).visit(testMethod, predicateInfo);
    if (!onlyForOneEntity && !testedMethod.empty() && !predicateInfo.has_value()) {
        string assumption = concat("(", PrinterUtils::KLEE_PATH_FLAG, PrinterUtils::EQ_OPERATOR, PrinterUtils::KLEE_PATH_FLAG_SYMBOLIC, ") & (",
                                   PrinterUtils::KLEE_PATH_FLAG_SYMBOLIC, PrinterUtils::EQ_OPERATOR, "1)");
        strFunctionCall(PrinterUtils::KLEE_ASSUME, { assumption });
    }
}

void KleePrinter::genConstraints(const Tests::MethodParam &param, const string& methodName) {
    KleeConstraintsPrinter constraintsPrinter(typesHandler, srcLanguage);
    constraintsPrinter.setTabsDepth(tabsDepth);
    const auto constraintsBlock = constraintsPrinter.genConstraints(param).str();
    ss << constraintsBlock;
}

void KleePrinter::genKleePathSymbolicIfNeeded(
    const std::optional<LineInfo::PredicateInfo> &predicateInfo,
    const string &testedMethod,
    bool onlyForOneEntity) {
    if (!predicateInfo.has_value() && !onlyForOneEntity && !testedMethod.empty()) {
        strDeclareVar("int", PrinterUtils::KLEE_PATH_FLAG_SYMBOLIC);
        strKleeMakeSymbolic(PrinterUtils::KLEE_PATH_FLAG_SYMBOLIC, true);
    }
}

[[maybe_unused]] void KleePrinter::addHeaderIncludeIfNecessary(std::unordered_set<string> &headers,
                                                               const types::Type &type) {
    const types::Type baseType = type.baseTypeObj();
    if (typesHandler->isStruct(baseType)) {
        auto filepath = typesHandler->getStructInfo(baseType).filePath;
        headers.insert(typesHandler->getStructInfo(baseType).filePath);
    }
    if (typesHandler->isEnum(baseType)) {
        auto filepath = typesHandler->getEnumInfo(baseType).filePath;
        headers.insert(typesHandler->getEnumInfo(baseType).filePath);
    }
    if (typesHandler->isUnion(baseType)) {
        headers.insert(typesHandler->getUnionInfo(baseType).filePath);
    }
}

KleePrinter::Stream KleePrinter::strKleeMakeSymbolic(const string &varName, bool needAmpersand) {
    return Printer::strKleeMakeSymbolic(varName, needAmpersand, varName);
}

KleePrinter::Stream KleePrinter::strKleeMakeSymbolic(const types::Type &type,
                                                     SRef varName,
                                                     SRef pseudoName,
                                                     bool needAmpersand) {
    if (type.isPointerToFunction()) {
        return ss;
    }
    return Printer::strKleeMakeSymbolic(varName, needAmpersand, pseudoName);
}

KleePrinter::Stream
KleePrinter::strKleeMakeSymbolic(const types::Type &type, SRef varName, bool needAmpersand) {
    if (type.isPointerToFunction()) {
        return ss;
    }
    return Printer::strKleeMakeSymbolic(varName, needAmpersand, varName);
}

void KleePrinter::genKleePathSymbolicAssumeIfNeeded(const std::optional<PredInfo> &predicateInfo,
                                                    const string &testedMethod,
                                                    bool onlyForOneEntity) {
    if (!onlyForOneEntity && !testedMethod.empty() && !predicateInfo.has_value()) {
        strFunctionCall(PrinterUtils::KLEE_ASSUME, { concat("(", PrinterUtils::KLEE_PATH_FLAG,
                                                            PrinterUtils::EQ_OPERATOR,
                                                            PrinterUtils::KLEE_PATH_FLAG_SYMBOLIC,
                                                            ") & (",
                                                            PrinterUtils::KLEE_PATH_FLAG_SYMBOLIC,
                                                            PrinterUtils::EQ_OPERATOR,
                                                            "1)") });
    }
}

void printer::KleePrinter::genTwoDimPointers(const Tests::MethodParam &param, bool needDeclare) {
    gen2DPointer(param, needDeclare);
}

utbot::Language printer::KleePrinter::getLanguage() const {
    return srcLanguage;
}