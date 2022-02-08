/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "TestsPrinter.h"

#include "Paths.h"
#include "utils/ArgumentsUtils.h"
#include "utils/Copyright.h"
#include "visitors/ParametrizedAssertsVisitor.h"
#include "visitors/VerboseAssertsParamVisitor.h"
#include "visitors/VerboseAssertsReturnValueVisitor.h"
#include "visitors/VerboseParameterVisitor.h"
#include "utils/KleeUtils.h"

using printer::TestsPrinter;

TestsPrinter::TestsPrinter(const types::TypesHandler *typesHandler, utbot::Language srcLanguage) : Printer(srcLanguage) , typesHandler(typesHandler) {
}

//we need this header for tests with generated NAN and INFINITY parameters to be compilable
bool TestsPrinter::needsMathHeader(const Tests &tests) {
    for (const auto &[methodName, methodDescription] : tests.methods) {
        for (const auto &methodTestCase : methodDescription.testCases) {
            for (const auto &paramValue : methodTestCase.paramValues) {
                if (paramValue.view->containsFPSpecialValue()) {
                    return true;
                }
            }
            for (const auto &paramValue : methodTestCase.globalPreValues) {
                    if (paramValue.view->containsFPSpecialValue()) {
                    return true;
                }
            }
            for (const auto &paramValue : methodTestCase.globalPostValues) {
                    if (paramValue.view->containsFPSpecialValue()) {
                    return true;
                }
            }
        }
    }
    return false;
}

void TestsPrinter::joinToFinalCode(Tests &tests, const fs::path& generatedHeaderPath) {
    resetStream();
    writeCopyrightHeader();
    genHeaders(tests, generatedHeaderPath);
    ss << "namespace " << PrinterUtils::TEST_NAMESPACE << " {\n";

    strDeclareAbsError(PrinterUtils::ABS_ERROR);

    for (const auto &commentBlock : tests.commentBlocks) {
        strComment(commentBlock) << NL;
    }
    writeStubsForStructureFields(tests);
    ss << NL;
    ss << RB();
    tests.code = ss.str();
    tests.regressionMethodsNumber = printSuiteAndReturnMethodsCount(Tests::DEFAULT_SUITE_NAME, tests.methods);
    tests.errorMethodsNumber = printSuiteAndReturnMethodsCount(Tests::ERROR_SUITE_NAME, tests.methods);
}

std::uint32_t TestsPrinter::printSuiteAndReturnMethodsCount(const string &suiteName, const Tests::MethodsMap &methods) {
    if (std::all_of(methods.begin(), methods.end(), [&suiteName](const auto& method) {
        return method.second.codeText.at(suiteName).empty();
    })) {
        return 0;
    }
    ss << "#pragma region " << suiteName << NL;
    std::uint32_t count = 0;
    for (const auto &[methodName, methodStub] : methods) {
        if (methodStub.codeText.at(suiteName).empty()) {
            continue;
        }
        ++count;
        ss << methodStub.codeText.at(suiteName);
    }
    ss << "#pragma endregion" << NL;
    return count;
}

void TestsPrinter::genCode(Tests::MethodDescription &methodDescription,
                           const std::optional<LineInfo::PredicateInfo>& predicateInfo,
                           bool verbose) {
    resetStream();

    if(needDecorate()) {
        methodDescription.name = KleeUtils::getRenamedOperator(methodDescription.name);
    }

    int testNum = 0;

    writeStubsForFunctionParams(typesHandler, methodDescription, false);
    writeExternForSymbolicStubs(methodDescription);

    genCodeBySuiteName(Tests::DEFAULT_SUITE_NAME,
                       methodDescription,
                       predicateInfo,
                       verbose,
                       testNum);
    resetStream();
    genCodeBySuiteName(Tests::ERROR_SUITE_NAME,
                       methodDescription,
                       predicateInfo,
                       verbose,
                       testNum);
    resetStream();
}

void TestsPrinter::genCodeBySuiteName(const string &targetSuiteName,
                                      Tests::MethodDescription &methodDescription,
                                      const std::optional<LineInfo::PredicateInfo>& predicateInfo,
                                      bool verbose,
                                      int& testNum) {
    const auto& testCases = methodDescription.suiteTestCases[targetSuiteName];
    if (testCases.empty()) {
        return;
    }
    for (auto &testCase : testCases) {
        testNum++;
        testHeader(testCase.suiteName, methodDescription, testNum);
        redirectStdin(methodDescription, testCase, verbose);
        if (verbose) {
            genVerboseTestCase(methodDescription, testCase, predicateInfo);
        } else {
            genParametrizedTestCase(methodDescription, testCase, predicateInfo);
        }
    }

    methodDescription.codeText[targetSuiteName] = ss.str();
}

void TestsPrinter::genVerboseTestCase(const Tests::MethodDescription &methodDescription,
                                      const Tests::MethodTestCase &testCase,
                                      std::optional<LineInfo::PredicateInfo> predicateInfo) {
    TestsPrinter::verboseParameters(methodDescription, testCase);
    ss << NL;
    if (!testCase.isError()) {
        TestsPrinter::verboseOutputVariable(methodDescription, testCase);
        ss << NL;
    }
    TestsPrinter::verboseFunctionCall(methodDescription, testCase);
    ss << NL;
    if (testCase.isError()) {
        ss << TAB_N()
           << "FAIL() << \"Unreachable point. "
              "Function was supposed to fail, but actually completed successfully.\""
           << SCNL;
    } else {
        TestsPrinter::verboseAsserts(methodDescription, testCase, predicateInfo);
    }
    ss << RB() << NL;
}

void TestsPrinter::printLazyVariables(const Tests::MethodDescription &methodDescription,
                                      const Tests::MethodTestCase &testCase) {
    for (auto i = 0; i < testCase.lazyValues.size(); i++) {
        const auto &param = testCase.lazyVariables[i];
        const auto &value = testCase.lazyValues[i];
        if (param.type.isObjectPointer()) {
                strDeclareVar(param.type.baseType(), param.varName, value.view->getEntryValue());
        }
    }
}

void TestsPrinter::printLazyReferences(const Tests::MethodDescription &methodDescription,
                                       const Tests::MethodTestCase &testCase) {
    for (const auto &lazy : testCase.lazyReferences) {
        strAssignVar(lazy.varName, "&" + lazy.refName);
    }
}

void TestsPrinter::printStubVariables(const Tests::MethodDescription &methodDescription,
                                      const Tests::MethodTestCase &testCase) {
    for (int i = 0; i < testCase.stubParamValues.size(); i++) {
        auto stub = testCase.stubParamValues[i];
        types::Type stubType = testCase.stubParamTypes[i].type;
        std::string bufferSuffix = "_buffer";
        std::string buffer = stub.name + bufferSuffix;
        strDeclareArrayVar(stubType, buffer, types::PointerUsage::PARAMETER, stub.view->getEntryValue());
        strMemcpy(stub.name, buffer, false);
    }
}

void TestsPrinter::genParametrizedTestCase(const Tests::MethodDescription &methodDescription,
                                           const Tests::MethodTestCase &testCase,
                                           const std::optional<LineInfo::PredicateInfo>& predicateInfo) {
    parametrizedInitializeGlobalVariables(methodDescription, testCase);
    parametrizedInitializeSymbolicStubs(methodDescription, testCase);
    parametrizedArrayParameters(methodDescription, testCase);
    printClassObject(methodDescription, testCase);
    printLazyVariables(methodDescription, testCase);
    printLazyReferences(methodDescription, testCase);
    printStubVariables(methodDescription, testCase);
    printFunctionParameters(methodDescription, testCase, true);
    parametrizedAsserts(methodDescription, testCase, predicateInfo);
    ss << RB() << NL;
}

void TestsPrinter::genHeaders(Tests &tests, const fs::path& generatedHeaderPath) {
    strInclude(generatedHeaderPath.filename()) << NL;

    strInclude("gtest/gtest.h");

    if (needsMathHeader(tests)) {
        LOG_S(INFO) << "Added extra \"math.h\" include to file " << tests.testFilename;
        tests.srcFileHeaders.emplace_back(false, Paths::mathIncludePath().string());
    }

    for (const auto &header : tests.srcFileHeaders) {
        if (header.is_angled) {
            strIncludeSystem(header.path);
        } else {
            strInclude(header.path);
        }
    }

    writeAccessPrivateMacros(typesHandler, tests, true);
}

static string getTestName(const Tests::MethodDescription &methodDescription, int testNum) {
    string renamedMethodDescription = KleeUtils::getRenamedOperator(methodDescription.name);
    string testBaseName = methodDescription.isClassMethod()
        ? StringUtils::stringFormat("%s_%s", methodDescription.classObj->type.typeName(),
                                    renamedMethodDescription)
        : renamedMethodDescription;

    return printer::Printer::concat(testBaseName, "_test_", testNum);
}

void TestsPrinter::testHeader(const string &scopeName,
                              const Tests::MethodDescription &methodDescription,
                              int testNum) {
    string testName = getTestName(methodDescription, testNum);
    strFunctionCall("TEST", { scopeName, testName }, NL) << LB(false);
}

void TestsPrinter::redirectStdin(const Tests::MethodDescription &methodDescription,
                                 const Tests::MethodTestCase &testCase,
                                 bool verbose) {
    if (testCase.stdinValue == std::nullopt) {
        return;
    }
    if (verbose) {
        strComment("Redirect stdin");
    }
    const types::Type stdinBufferType =
        Tests::getStdinMethodParam().type.arrayClone(types::PointerUsage::RETURN);
    visitor::VerboseParameterVisitor(typesHandler, this, true, types::PointerUsage::RETURN)
        .visit(stdinBufferType, types::Type::getStdinParamName(), testCase.stdinValue.value().view.get(),
               std::nullopt);
    string utbotRedirectStdinStatus = "utbot_redirect_stdin_status";
    auto view = tests::JustValueView("0");
    visitor::VerboseParameterVisitor(typesHandler, this, true, types::PointerUsage::RETURN)
        .visit(types::Type::intType(), utbotRedirectStdinStatus, &view, std::nullopt);
    strFunctionCall("utbot_redirect_stdin", { types::Type::getStdinParamName(), utbotRedirectStdinStatus });
    strIfBound("utbot_redirect_stdin_status != 0") << LB();
    ss << TAB_N() << "FAIL() << \"Unable to redirect stdin.\"" << SCNL;
    ss << RB();
}

void TestsPrinter::verboseParameters(const Tests::MethodDescription &methodDescription,
                                     const Tests::MethodTestCase &testCase) {
    if (!methodDescription.globalParams.empty()) {
        strComment("Initialize global variables");
        for (auto i = 0; i < methodDescription.globalParams.size(); i++) {
            const auto &param = methodDescription.globalParams[i];
            const auto &value = testCase.globalPreValues[i];
            if (param.type.isTwoDimensionalPointer()) {
                Tests::MethodParam valueParam{ param.type, param.underscoredName(), param.alignment };
                verboseParameter(methodDescription, valueParam, value, true);
                gen2DPointer(param, false);
            } else {
                verboseParameter(methodDescription, param, value, false);
            }
        }
        ss << NL;
    }

    if (!testCase.stubValuesTypes.empty()) {
        strComment("Initialize symbolic stubs");
        for (auto i = 0; i < testCase.stubValuesTypes.size(); i++) {
            const auto &param = testCase.stubValuesTypes[i];
            const auto &value = testCase.stubValues[i];
            if (param.type.isTwoDimensionalPointer()) {
                Tests::MethodParam valueParam{ param.type, param.underscoredName(), param.alignment };
                verboseParameter(methodDescription, valueParam, value, true);
                gen2DPointer(param, false);
            } else {
                verboseParameter(methodDescription, param, value, false);
            }
        }
        ss << NL;
    }

    if (!testCase.paramValues.empty()) {
        strComment("Construct input");
    }
    printClassObject(methodDescription, testCase);
    printFunctionParameters(methodDescription, testCase, false);
}

void TestsPrinter::printFunctionParameters(const Tests::MethodDescription &methodDescription,
                                             const Tests::MethodTestCase &testCase,
                                             bool onlyLValue){
    for (auto i = 0; i < testCase.paramValues.size(); i++) {
        if(!onlyLValue || methodDescription.params[i].type.isLValueReference()) {
            Tests::MethodParam param = methodDescription.params[i];
            auto value = testCase.paramValues[i];
            Tests::MethodParam valueParam = getValueParam(param);
            if (methodDescription.params[i].type.maybeJustPointer()) {
                valueParam.type = valueParam.type.baseTypeObj();
            }
            value.name = valueParam.name;
            verboseParameter(methodDescription, valueParam, value, true);
            if (param.type.isTwoDimensionalPointer()) {
                gen2DPointer(param, true);
            }
        }
    }
}

void TestsPrinter::verboseParameter(const Tests::MethodDescription &method,
                                    const Tests::MethodParam &param,
                                    const Tests::TestCaseParamValue &value,
                                    bool needDeclaration) {
    string stubFunctionName =
        PrinterUtils::getFunctionPointerStubName(method.getClassTypeName(),
                                                 method.name, param.name);
    if (types::TypesHandler::isPointerToFunction(param.type)) {
        strDeclareVar(getTypedefFunctionPointer(method.name, param.name, false), param.name,
                      stubFunctionName);
    } else if (types::TypesHandler::isArrayOfPointersToFunction(param.type)) {
        strDeclareArrayOfFunctionPointerVar(getTypedefFunctionPointer(method.name, param.name, false), param.name, stubFunctionName);
    } else {
        auto paramType = types::TypesHandler::isVoid(param.type) ? types::Type::minimalScalarType() : param.type;
        visitor::VerboseParameterVisitor(typesHandler, this, needDeclaration, types::PointerUsage::PARAMETER)
            .visit(paramType, param.name, value.view.get(), param.alignment);
    }
}

void printer::TestsPrinter::printClassObject(const Tests::MethodDescription &methodDescription,
                                             const Tests::MethodTestCase &testCase) {
    if (methodDescription.isClassMethod()) {
        const auto &param = methodDescription.classObj.value();
        const auto &value = testCase.classPreValues.value();
        verboseParameter(methodDescription, param, value, true);
    }
}

void TestsPrinter::verboseOutputVariable(const Tests::MethodDescription &methodDescription,
                                         const Tests::MethodTestCase &testCase) {
    const types::Type baseReturnType = methodDescription.returnType.baseTypeObj();
    const types::Type expectedType = methodDescription.returnType.maybeReturnArray() ?
        methodDescription.returnType.arrayClone(types::PointerUsage::RETURN) :
        typesHandler->getReturnTypeToCheck(methodDescription.returnType);
    strComment("Expected output");

    if (types::TypesHandler::isVoid(methodDescription.returnType)) {
        strComment("No output variable for void function");
    } else if (types::TypesHandler::isPointerToFunction(methodDescription.returnType) ||
               types::TypesHandler::isArrayOfPointersToFunction(methodDescription.returnType)) {
        strComment("No output variable check for function returning pointer to function");
    } else if (methodDescription.returnType.isObjectPointer() &&
               testCase.returnValueView->getEntryValue() == PrinterUtils::C_NULL) {
        strComment("No output variable check for function returning null");
    } else {
        visitor::VerboseParameterVisitor(typesHandler, this, true, types::PointerUsage::RETURN)
            .visit(expectedType, PrinterUtils::EXPECTED, testCase.returnValueView.get(), std::nullopt);
    }
}

void TestsPrinter::verboseFunctionCall(const Tests::MethodDescription &methodDescription,
                                       const Tests::MethodTestCase &testCase) {
    std::string baseReturnType = types::TypesHandler::cBoolToCpp(methodDescription.returnType.baseType());
    types::Type expectedType = typesHandler->getReturnTypeToCheck(methodDescription.returnType);
    if (methodDescription.returnType.maybeReturnArray()) {
        expectedType = methodDescription.returnType.arrayClone(types::PointerUsage::RETURN);
    }
    strComment("Trigger the function");
    string methodCall = constrVisitorFunctionCall(methodDescription, testCase, true);
    if (!types::TypesHandler::skipTypeInReturn(methodDescription.returnType) && !testCase.isError()) {
        size_t returnPointersCount = 0;
        if (testCase.returnValueView->getEntryValue() == PrinterUtils::C_NULL) {
            returnPointersCount = methodDescription.returnType.countReturnPointers(true);
        }
        auto type = Printer::getConstQualifier(expectedType) + expectedType.usedType();
        strDeclareVar(type, PrinterUtils::ACTUAL, methodCall, std::nullopt, true, returnPointersCount);
    } else {
        ss << TAB_N() << methodCall << SCNL;
    }
}

void TestsPrinter::verboseAsserts(const Tests::MethodDescription &methodDescription,
                                  const Tests::MethodTestCase &testCase,
                                  std::optional<LineInfo::PredicateInfo> predicateInfo) {
    strComment("Check results");
    if (types::TypesHandler::isVoid(methodDescription.returnType)) {
        strComment("No check results for void function");
    } else if (types::TypesHandler::isPointerToFunction(methodDescription.returnType) ||
               types::TypesHandler::isArrayOfPointersToFunction(methodDescription.returnType)) {
        strComment("No check results for function returning pointer to function");
    } else {
        auto visitor = visitor::VerboseAssertsReturnValueVisitor(typesHandler, this, predicateInfo);
        visitor.visit(methodDescription, testCase);
    }

    if (!methodDescription.globalParams.empty()) {
        ss << NL;
        strComment("Check global variables");
        globalParamsAsserts(methodDescription, testCase);
    }

    if (methodDescription.isClassMethod()) {
        ss << NL;
        strComment("Check class fields mutation");
        classAsserts(methodDescription, testCase);
    }

    if (!testCase.paramPostValues.empty()) {
        ss << NL;
        strComment("Check function parameters");
        changeableParamsAsserts(methodDescription, testCase);
    }
}

void TestsPrinter::classAsserts(const Tests::MethodDescription &methodDescription,
                                           const Tests::MethodTestCase &testCase) {
    if (methodDescription.isClassMethod()) {
        auto parameterVisitor = visitor::VerboseParameterVisitor(typesHandler, this, true,
                                                                 types::PointerUsage::PARAMETER);
        auto assertsVisitor = visitor::VerboseAssertsParamVisitor(typesHandler, this);
        auto usage = types::PointerUsage::PARAMETER;
        size_t param_i = 0;
        auto param = methodDescription.classObj.value();
        auto const &value = testCase.classPostValues.value();
        string expectedName = PrinterUtils::getExpectedVarName(param.name);
        const types::Type expectedType = param.type.arrayCloneMultiDim(usage);
        parameterVisitor.visit(expectedType, expectedName, value.view.get(), std::nullopt);
        assertsVisitor.visit(param, param.name);
    }
}

void TestsPrinter::changeableParamsAsserts(const Tests::MethodDescription &methodDescription,
                                 const Tests::MethodTestCase &testCase){
    auto parameterVisitor = visitor::VerboseParameterVisitor(typesHandler, this, true, types::PointerUsage::PARAMETER);
    auto assertsVisitor = visitor::VerboseAssertsParamVisitor(typesHandler, this);
    auto usage = types::PointerUsage::PARAMETER;
    size_t param_i = 0;
    for (const auto& param : methodDescription.params) {
        if (param.isChangeable()) {
            auto const &value = testCase.paramPostValues[param_i];
            string expectedName = PrinterUtils::getExpectedVarName(param.name);
            const types::Type expectedType = param.type.arrayCloneMultiDim(usage);
            parameterVisitor.visit(expectedType, expectedName, value.view.get(), std::nullopt);
            assertsVisitor.visit(param, param.name);
            param_i++;
        }
    }
}

void TestsPrinter::globalParamsAsserts(const Tests::MethodDescription &methodDescription,
                                       const Tests::MethodTestCase &testCase){

    auto parameterVisitor = visitor::VerboseParameterVisitor(typesHandler, this, true, types::PointerUsage::PARAMETER);
    auto assertsVisitor = visitor::VerboseAssertsParamVisitor(typesHandler, this);
    for (size_t i = 0; i < methodDescription.globalParams.size(); i++) {
        auto const &param = methodDescription.globalParams[i];
        auto const &value = testCase.globalPostValues[i];
        string expectedName = PrinterUtils::getExpectedVarName(param.name);
        auto expectedType = typesHandler->getReturnTypeToCheck(param.type);
        Tests::MethodParam expectedParam{expectedType, expectedName, param.alignment};
        parameterVisitor.visit(expectedParam.type, expectedParam.name, value.view.get(), std::nullopt);
        assertsVisitor.visitGlobal(param, param.name);
    }
}

void TestsPrinter::parametrizedArrayParameters(const Tests::MethodDescription &methodDescription,
                                               const Tests::MethodTestCase &testCase) {
    for (auto i = 0; i < testCase.paramValues.size(); i++) {
        const auto &param = methodDescription.params[i];
        const auto &value = testCase.paramValues[i];
        if (types::TypesHandler::isArrayOfPointersToFunction(param.type)) {
            auto type = getTypedefFunctionPointer(methodDescription.name, param.name, false);
            string stubName = PrinterUtils::getFunctionPointerStubName(
                methodDescription.getClassTypeName(), methodDescription.name, param.name);
            strDeclareArrayOfFunctionPointerVar(type, param.name, stubName);
        } else if (types::TypesHandler::isCStringType(param.type)) {
            strDeclareArrayVar(param.type, param.name, types::PointerUsage::PARAMETER, value.view->getEntryValue(), param.alignment);
        } else if (param.type.isObjectPointer() || param.type.isArray()) {
            auto arrayType = types::TypesHandler::isVoid(param.type.baseTypeObj())
                ? types::Type::minimalScalarPointerType(param.type.arraysSizes(types::PointerUsage::PARAMETER).size())
                : param.type;
            if (param.type.maybeJustPointer()) {
                strDeclareVar(arrayType.baseType(), param.name, value.view->getEntryValue(), param.alignment);
            } else {
                auto paramName = param.type.isTwoDimensionalPointer() ? param.underscoredName() : param.name;
                strDeclareArrayVar(arrayType, paramName, types::PointerUsage::PARAMETER, value.view->getEntryValue(), true);
            }
        }
        if (param.type.isTwoDimensionalPointer()) {
            gen2DPointer(param, true);
        }
    }
}

void TestsPrinter::parametrizedAsserts(const Tests::MethodDescription &methodDescription,
                                       const Tests::MethodTestCase &testCase,
                                       const std::optional<LineInfo::PredicateInfo>& predicateInfo) {
    auto visitor = visitor::ParametrizedAssertsVisitor(typesHandler, this, predicateInfo, testCase.isError());
    visitor.visit(methodDescription, testCase);
    globalParamsAsserts(methodDescription, testCase);
    classAsserts(methodDescription, testCase);
    changeableParamsAsserts(methodDescription, testCase);
}

std::vector<string> TestsPrinter::methodParametersListParametrized(const Tests::MethodDescription &methodDescription,
                                                                   const Tests::MethodTestCase &testCase) {
    std::vector<string> args;
    for (size_t i = 0; i < methodDescription.params.size(); ++i) {
        Tests::MethodParam const &param = methodDescription.params[i];
        if (param.type.isTwoDimensionalPointer() &&
            types::TypesHandler::isVoid(param.type.baseTypeObj())) {
            string qualifier = Printer::getConstQualifier(param.type);
            string arg = StringUtils::stringFormat("(%svoid **) %s", qualifier, param.name);
            args.push_back(arg);
        } else if (param.type.isObjectPointer() || param.type.isArray()) {
            string maybeAmpersand = param.type.maybeJustPointer() ? "&" : "";
            args.push_back(maybeAmpersand + param.name);
        } else if (param.type.isLValueReference()) {
            args.push_back(param.name);
        } else {
            args.push_back(testCase.paramValues[i].view->getEntryValue());
        }
    }
    return args;
}

std::vector<string>
TestsPrinter::methodParametersListVerbose(const Tests::MethodDescription &methodDescription,
                                          const Tests::MethodTestCase &testCase) {
    std::vector<string> args;
    for (const auto & param : methodDescription.params) {
        if (param.type.isTwoDimensionalPointer() &&
                types::TypesHandler::isVoid(param.type.baseTypeObj())) {
            string qualifier = Printer::getConstQualifier(param.type);
            string arg = StringUtils::stringFormat("(%svoid **) %s", qualifier, param.name);
            args.push_back(arg);
        } else {
            string maybeAmpersand = param.type.maybeJustPointer() ? "&" : "";
            args.push_back(maybeAmpersand + param.name);
        }
    }
    return args;
}

string TestsPrinter::constrVisitorFunctionCall(const Tests::MethodDescription &methodDescription,
                                               const Tests::MethodTestCase &testCase,
                                               bool verboseMode) {
    std::vector<string> methodArgs;
    if (verboseMode) {
        methodArgs = methodParametersListVerbose(methodDescription, testCase);
    } else {
        methodArgs = methodParametersListParametrized(methodDescription, testCase);
    }

    std::optional<types::Type> castType;
    if (types::TypesHandler::skipTypeInReturn(methodDescription.returnType.baseTypeObj()) &&
        methodDescription.returnType.isObjectPointer()) {
        castType = types::Type::minimalScalarPointerType();
    }
    auto classObjName = methodDescription.getClassName();
    size_t returnPointersCount = 0;
    if (testCase.returnValueView && testCase.returnValueView->getEntryValue() != PrinterUtils::C_NULL) {
        returnPointersCount = methodDescription.returnType.countReturnPointers(true);
    }
    return constrFunctionCall(methodDescription.name, methodArgs, "", classObjName, false, returnPointersCount,
                              castType);
}

void printer::TestsPrinter::parametrizedInitializeGlobalVariables(const Tests::MethodDescription &methodDescription,
                                                                  const Tests::MethodTestCase &testCase) {
    for (auto i = 0; i < methodDescription.globalParams.size(); i++) {
        const auto &param = methodDescription.globalParams[i];
        const auto &value = testCase.globalPreValues[i];
        verboseParameter(methodDescription, param, value, false);
    }
}

void printer::TestsPrinter::parametrizedInitializeSymbolicStubs(const Tests::MethodDescription &methodDescription,
                                                                const Tests::MethodTestCase &testCase) {
    for (auto i = 0; i < testCase.stubValues.size(); i++) {
        const auto &param = testCase.stubValuesTypes[i];
        const auto &value = testCase.stubValues[i];
        verboseParameter(methodDescription, param, value, false);
    }
}

Tests::MethodParam printer::TestsPrinter::getValueParam(const Tests::MethodParam &param) {
    if (param.type.isTwoDimensionalPointer()) {
        return { param.type, param.underscoredName(), param.alignment };
    } else {
        return param;
    }
}

utbot::Language printer::TestsPrinter::getLanguage() const {
    return utbot::Language::CXX;
}