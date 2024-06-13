#include "TestsPrinter.h"

#include "Paths.h"
#include "SARIFGenerator.h"
#include "utils/Copyright.h"
#include "utils/JsonUtils.h"
#include "visitors/ParametrizedAssertsVisitor.h"
#include "visitors/VerboseAssertsParamVisitor.h"
#include "visitors/VerboseAssertsReturnValueVisitor.h"
#include "visitors/VerboseParameterVisitor.h"
#include "utils/KleeUtils.h"
#include "utils/StubsUtils.h"

#include "loguru.h"

using json = nlohmann::json;
using printer::TestsPrinter;

TestsPrinter::TestsPrinter(const utbot::ProjectContext &projectContext,
                           const types::TypesHandler *typesHandler,
                           utbot::Language srcLanguage)
    : Printer(srcLanguage), projectContext(projectContext), typesHandler(typesHandler) {
}

bool TestsPrinter::paramNeedsMathHeader(const Tests::TestCaseParamValue &paramValue) {
    if (paramValue.view->containsFPSpecialValue()) {
        return true;
    }
    for (const auto &lazyParamValue : paramValue.lazyValues) {
        if (paramNeedsMathHeader(lazyParamValue)) {
            return true;
        }
    }
    return false;
}

//we need this header for tests with generated NAN and INFINITY parameters to be compilable
bool TestsPrinter::needsMathHeader(const Tests &tests) {
    for (const auto &[methodName, methodDescription] : tests.methods) {
        for (const auto &methodTestCase : methodDescription.testCases) {
            for (const auto &paramValue : methodTestCase.paramValues) {
                if (paramNeedsMathHeader(paramValue)) {
                    return true;
                }
            }
            for (const auto &paramValue : methodTestCase.globalPreValues) {
                if (paramNeedsMathHeader(paramValue)) {
                    return true;
                }
            }
            for (const auto &paramValue : methodTestCase.globalPostValues) {
                if (paramNeedsMathHeader(paramValue)) {
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
    ss << printer::NL;

    strDeclareSetOfExternVars(tests.externVariables);

    ss << "namespace " << PrinterUtils::TEST_NAMESPACE << " {\n";

    for (const auto &commentBlock : tests.commentBlocks) {
        strComment(commentBlock) << printer::NL;
    }
    writeStubsForStructureFields(tests);
    ss << printer::NL;
    writeStubsForParameters(tests);
    ss << printer::NL;

    tests.regressionMethodsNumber = printSuiteAndReturnMethodsCount(Tests::DEFAULT_SUITE_NAME, tests.methods);
    tests.errorMethodsNumber = printSuiteAndReturnMethodsCount(Tests::ERROR_SUITE_NAME, tests.methods);
    ss << RB();
    printFinalCodeAndAlterJson(tests);
}

void TestsPrinter::printFinalCodeAndAlterJson(Tests &tests) {
    int line_count = 0;
    std::string line;
    while (getline(ss, line)) {
        if (line.rfind(sarif::PREFIX_FOR_JSON_PATH, 0) != 0) {
            // ordinal string
            tests.code.append(line);
            tests.code.append("\n");
            ++line_count;
        } else {
            // anchor for SARIF
            std::string nameAndTestIndex =
                line.substr(sarif::PREFIX_FOR_JSON_PATH.size());
            size_t pos = nameAndTestIndex.find(',');
            if (pos != std::string::npos) {
                std::string name = nameAndTestIndex.substr(0, pos);
                int testIndex = -1;
                try {
                    testIndex = std::stoi(nameAndTestIndex.substr(pos + 1));
                } catch (std::logic_error &e) {
                    // ignore
                }
                Tests::MethodsMap::iterator it = tests.methods.find(name);
                if (it != tests.methods.end() && testIndex >= 0) {
                    Tests::MethodDescription &methodDescription = it.value();
                    std::vector<Tests::MethodTestCase> &testCases = methodDescription.testCases;
                    if (testIndex < testCases.size()) {
                        Tests::MethodTestCase &testCase = testCases[testIndex];
                        auto &descriptors = testCase.errorDescriptors;
                        if (testCase.errorDescriptors.empty()) {
                            LOG_S(ERROR) << "no error info for test case: "
                                         << name
                                         << ", test #"
                                         << testIndex;
                            continue;
                        }
                        std::stringstream ssFromTestCallInfo;
                        ssFromTestCallInfo
                          << sarif::TEST_FILE_KEY << ":" << tests.testSourceFilePath.c_str() << std::endl
                          << sarif::TEST_LINE_KEY << ":" << line_count << std::endl
                          << sarif::TEST_NAME_KEY << ":" << testCase.suiteName
                                                         << "."
                                                         << testCase.testName
                                                         << std::endl;

                        descriptors.emplace_back(ssFromTestCallInfo.str());
                        // ok
                        continue;
                    }
                }
            }
            LOG_S(ERROR) << "wrong SARIF anchor (need {testFilePath,lineThatCallsTestedFunction}): "
                         << line;
        }
    }
}

std::uint32_t
TestsPrinter::printSuiteAndReturnMethodsCount(const std::string &suiteName, const Tests::MethodsMap &methods) {
    if (std::all_of(methods.begin(), methods.end(), [&suiteName](const auto &method) {
        return method.second.codeText.at(suiteName).empty();
    })) {
        return 0;
    }
    ss << "#pragma region " << suiteName << printer::NL;
    std::uint32_t count = 0;
    for (const auto &[methodName, methodDescription]: methods) {
        if (methodDescription.codeText.at(suiteName).empty()) {
            continue;
        }
        count += methodDescription.suiteTestCases.at(suiteName).size();
        ss << methodDescription.codeText.at(suiteName);
    }
    ss << "#pragma endregion" << printer::NL;
    return count;
}

void TestsPrinter::genCode(Tests::MethodDescription &methodDescription,
                           const std::optional<LineInfo::PredicateInfo> &predicateInfo,
                           bool verbose,
                           ErrorMode errorMode) {
    resetStream();

    if (needDecorate()) {
        methodDescription.name = KleeUtils::getRenamedOperator(methodDescription.name);
    }

    int testNum = 0;

    writeStubsForFunctionParams(typesHandler, methodDescription, false);
    writeExternForSymbolicStubs(methodDescription);

    methodDescription.stubsText = ss.str();
    resetStream();

    genCodeBySuiteName(Tests::DEFAULT_SUITE_NAME,
                       methodDescription,
                       predicateInfo,
                       verbose,
                       testNum,
                       errorMode);
    resetStream();
    genCodeBySuiteName(Tests::ERROR_SUITE_NAME,
                       methodDescription,
                       predicateInfo,
                       verbose,
                       testNum,
                       errorMode);
    resetStream();
}

static std::string getTestName(const Tests::MethodDescription &methodDescription, int testNum) {
    std::string renamedMethodDescription = KleeUtils::getRenamedOperator(methodDescription.name);
    StringUtils::replaceColon(renamedMethodDescription);
    std::string testBaseName = methodDescription.isClassMethod()
                               ? StringUtils::stringFormat("%s_%s",
                                                           methodDescription.classObj->type.typeName(),
                                                           renamedMethodDescription)
                               : renamedMethodDescription;

    return printer::Printer::concat(testBaseName, Paths::TEST_SUFFIX, testNum);
}

void TestsPrinter::genCodeBySuiteName(const std::string &targetSuiteName,
                                      Tests::MethodDescription &methodDescription,
                                      const std::optional<LineInfo::PredicateInfo> &predicateInfo,
                                      bool verbose,
                                      int &testNum,
                                      ErrorMode errorMode) {
    const auto &testCases = methodDescription.suiteTestCases[targetSuiteName];
    if (testCases.empty()) {
        return;
    }
    for (int testCaseIndex: testCases) {
        ++testNum;
        Tests::MethodTestCase &testCase = methodDescription.testCases[testCaseIndex];
        testCase.testName = getTestName(methodDescription, testNum);
        testHeader(testCase);
        redirectStdin(methodDescription, testCase, verbose);
        if (verbose) {
            genVerboseTestCase(methodDescription, testCase, predicateInfo, errorMode);
        } else {
            genParametrizedTestCase(methodDescription, testCase, predicateInfo, errorMode);
        }
        genTearDownCall(methodDescription);
        ss << RB() << printer::NL;
    }

    methodDescription.codeText[targetSuiteName] = ss.str();
}

void TestsPrinter::genVerboseTestCase(const Tests::MethodDescription &methodDescription,
                                      const Tests::MethodTestCase &testCase,
                                      const std::optional<LineInfo::PredicateInfo> &predicateInfo,
                                      ErrorMode errorMode) {
    initializeFiles(methodDescription, testCase);
    openFiles(methodDescription, testCase);

    TestsPrinter::verboseParameters(methodDescription, testCase);

    printLazyVariables(methodDescription, testCase, true);

    printLazyReferences(methodDescription, testCase, true);

    if (!testCase.isError()) {
        TestsPrinter::verboseOutputVariable(methodDescription, testCase);
    }
    if (testCase.errorInfo.errorType == ErrorType::ASSERTION_FAILURE) {
        ss << LINE_INDENT() << "/*"
           << LINE_INDENT() << testCase.errorInfo.failureBody << printer::NL
           << LINE_INDENT() << "FILE: " << testCase.errorInfo.fileWithFailure.string() << printer::NL
           << LINE_INDENT() << "LINE: " << testCase.errorInfo.lineWithFailure << printer::NL
           << LINE_INDENT() << "*/" << printer::NL;
    }

    TestsPrinter::verboseFunctionCall(methodDescription, testCase, errorMode);
    markTestedFunctionCallIfNeed(methodDescription.name, testCase);

    if (testCase.isError()) {
        printFailAssertion(errorMode);
    } else {
        ss << printer::NL;
        TestsPrinter::verboseAsserts(methodDescription, testCase, predicateInfo);
    }
}

void TestsPrinter::initializeFiles(const Tests::MethodDescription &methodDescription,
                                   const Tests::MethodTestCase &testCase) {
    if (!testCase.filesValues.has_value()) {
        LOG_S(INFO) << "There are not symbolic files in the test.";
        return;
    }
    fs::path pathToSourceFile =
        Paths::sourcePathToTestPath(projectContext, methodDescription.sourceFilePath);
    fs::path pathToTestDir = Paths::getPathDirRelativeToBuildDir(projectContext, pathToSourceFile);
    int numInitFiles = 0;
    for (char fileName = 'A'; fileName < 'A' + types::Type::symFilesCount; fileName++) {
        if (testCase.getFileByName(fileName).readBytes == 0) {
            continue;
        }

        numInitFiles++;
        std::string strFileName(1, fileName);
        strFunctionCall("write_to_file", { StringUtils::wrapQuotations(pathToTestDir / strFileName),
                                           testCase.getFileByName(fileName).data });
    }
    if (numInitFiles != 0) {
        ss << printer::NL;
    }
}

void TestsPrinter::openFiles(const Tests::MethodDescription &methodDescription,
                             const Tests::MethodTestCase &testCase) {
    if (!testCase.filesValues.has_value()) {
        return;
    }
    char fileName = 'A';
    fs::path pathToSourceFile =
        Paths::sourcePathToTestPath(projectContext, methodDescription.sourceFilePath);
    fs::path pathToTestDir = Paths::getPathDirRelativeToBuildDir(projectContext, pathToSourceFile);

    for (auto &param : methodDescription.params) {
        if (!param.type.isFilePointer()) {
            continue;
        }

        std::string strFileName(1, fileName);
        std::string fileMode =
            testCase.getFileByName(fileName).writeBytes > 0 ? "\"w\"" : "\"r\"";
        strDeclareVar(param.type.typeName(), param.name,
                      constrFunctionCall(
                          "(UTBot::FILE *) fopen",
                          { StringUtils::wrapQuotations(pathToTestDir / strFileName), fileMode },
                          "", std::nullopt, false));
        fileName++;
    }
    if (fileName != 'A') {
        ss << printer::NL;
    }
}

void TestsPrinter::printLazyVariables(const Tests::MethodDescription &methodDescription,
                                      const Tests::MethodTestCase &testCase,
                                      bool verbose) {
    if (!testCase.lazyReferences.empty()) {
        if (verbose) {
            strComment("Construct lazy instantiated variables");
        }
        for (const auto &paramValue : testCase.paramValues) {
            printLazyVariables(paramValue.lazyParams, paramValue.lazyValues);
        }
        ss << printer::NL;
    }
}

void TestsPrinter::printLazyVariables(const std::vector<Tests::MethodParam> &lazyParams,
                                      const std::vector<Tests::TestCaseParamValue> &lazyValues) {
    for (size_t i = 0; i < lazyParams.size(); ++i) {
        printLazyVariables(lazyValues[i].lazyParams, lazyValues[i].lazyValues);
        strDeclareVar(lazyParams[i].type.baseType(), lazyValues[i].name, lazyValues[i].view->getEntryValue(this),
                          std::nullopt, true, lazyParams[i].type.getDimension());
    }
}

void TestsPrinter::printLazyReferences(const Tests::MethodDescription &methodDescription,
                                       const Tests::MethodTestCase &testCase,
                                       bool verbose) {
    if (!testCase.lazyReferences.empty()) {
        if (verbose) {
            strComment("Assign lazy variables to pointer");
        }
        for (const auto &lazy : testCase.lazyReferences) {
            strAssignVar(lazy.varName, lazy.typeName);
        }
        ss << printer::NL;
    }
}

void TestsPrinter::printStubVariablesForParam(const Tests::MethodDescription &methodDescription,
                                              const Tests::MethodTestCase &testCase) {
    for (int i = 0; i < testCase.stubParamValues.size(); i++) {
        auto stub = testCase.stubParamValues[i];
        types::Type stubType = testCase.stubParamTypes[i].type;
        std::string bufferSuffix = "_buffer";
        std::string buffer = stub.name + bufferSuffix;
        strDeclareArrayVar(stubType, buffer, types::PointerUsage::PARAMETER, stub.view->getEntryValue(this));
        strMemcpy(stub.name, buffer, false);
    }
}

void TestsPrinter::genParametrizedTestCase(const Tests::MethodDescription &methodDescription,
                                           const Tests::MethodTestCase &testCase,
                                           const std::optional<LineInfo::PredicateInfo>& predicateInfo,
                                           ErrorMode errorMode) {
    initializeFiles(methodDescription, testCase);
    openFiles(methodDescription, testCase);
    parametrizedInitializeGlobalVariables(methodDescription, testCase);
    genInitCall(methodDescription);
    parametrizedInitializeSymbolicStubs(methodDescription, testCase);
    printStubVariablesForParam(methodDescription, testCase);
    printClassObject(methodDescription, testCase);
    printFunctionParameters(methodDescription, testCase, false);
    printLazyVariables(methodDescription, testCase, false);
    printLazyReferences(methodDescription, testCase, false);
    parametrizedAsserts(methodDescription, testCase, predicateInfo, errorMode);
}

void TestsPrinter::genHeaders(Tests &tests, const fs::path& generatedHeaderPath) {
    strInclude(generatedHeaderPath.filename()) << printer::NL;

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

void TestsPrinter::testHeader(const Tests::MethodTestCase &testCase) {
    if (testCase.isError()) {
        strComment(testCase.getError());
    }
    strFunctionCall("TEST", { testCase.suiteName, testCase.testName }, NL) << LB(false);
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
    std::string utbotRedirectStdinStatus = "utbot_redirect_stdin_status";
    auto view = tests::JustValueView("0");
    visitor::VerboseParameterVisitor(typesHandler, this, true, types::PointerUsage::RETURN)
        .visit(types::Type::intType(), utbotRedirectStdinStatus, &view, std::nullopt);
    strFunctionCall("utbot_redirect_stdin", { types::Type::getStdinParamName(), utbotRedirectStdinStatus });
    strIfBound("utbot_redirect_stdin_status != 0") << LB();
    ss << LINE_INDENT() << "FAIL() << \"Unable to redirect stdin.\"" << SCNL;
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
                Tests::MethodParam valueParam{param.type, param.underscoredName(), param.alignment };
                verboseParameter(methodDescription, valueParam, value, true);
                gen2DPointer(param, false);
            } else {
                verboseParameter(methodDescription, param, value, false);
            }
        }
        ss << printer::NL;
    }

    genInitCall(methodDescription);

    std::vector<std::vector<tests::Tests::MethodParam>> types = {testCase.stubValuesTypes, testCase.stubParamTypes};
    std::vector<std::vector<tests::Tests::TestCaseParamValue>> values = {testCase.stubValues, testCase.stubParamValues};

    for (int j = 0; j < types.size(); j++) {
        if (!types[j].empty()) {
            if (j == 0) {
                strComment("Initialize symbolic stubs");
            }
            for (auto i = 0; i < types[j].size(); i++) {
                const auto &param = types[j][i];
                const auto &value = values[j][i];
                if (param.type.isTwoDimensionalPointer()) {
                    Tests::MethodParam valueParam{param.type, param.underscoredName(), param.alignment };
                    verboseParameter(methodDescription, valueParam, value, true);
                    gen2DPointer(param, false);
                } else {
                    verboseParameter(methodDescription, param, value, false);
                }
            }
            ss << printer::NL;
        }
    }

    if (!testCase.paramValues.empty()) {
        strComment("Construct input");
    }
    printClassObject(methodDescription, testCase);
    printFunctionParameters(methodDescription, testCase, true);
    ss << printer::NL;
}

void TestsPrinter::printFunctionParameters(const Tests::MethodDescription &methodDescription,
                                           const Tests::MethodTestCase &testCase,
                                           bool all) {
    for (auto i = 0; i < testCase.paramValues.size(); i++) {
        printPointerParameter(methodDescription, testCase, i);
        const Tests::MethodParam &param = methodDescription.params[i];
        bool containsLazy = !testCase.paramValues[i].lazyValues.empty() && !param.isChangeable();
        if (!param.type.isFilePointer() &&
            (all || param.type.isLValueReference() || param.type.isSimple() && containsLazy)) {
            Tests::TestCaseParamValue value = testCase.paramValues[i];
            Tests::MethodParam valueParam = getValueParam(param);
            value.name = valueParam.name;
            if (param.type.isLValueReference() || param.type.isSimple() || param.type.isPointerToFunction()) {
                verboseParameter(methodDescription, valueParam, value, true);
            }
        }
    }
}

void TestsPrinter::verboseParameter(const Tests::MethodDescription &method,
                                    const Tests::MethodParam &param,
                                    const Tests::TestCaseParamValue &value,
                                    bool needDeclaration) {
    std::string stubFunctionName = StubsUtils::getFunctionPointerStubName(method.getClassTypeName(),
                                                                          method.name, param.name, false);
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
        if (!typesHandler->getStructInfo(param.type).isCLike) {
            strComment("struct/class maybe can't be construct");
        }
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
               testCase.returnValue.view->getEntryValue(nullptr) == PrinterUtils::C_NULL) {
        strComment("No output variable check for function returning null");
    } else {
        visitor::VerboseParameterVisitor(typesHandler, this, true, types::PointerUsage::RETURN)
            .visit(expectedType, PrinterUtils::EXPECTED, testCase.returnValue.view.get(), std::nullopt);
    }
    ss << printer::NL;
}

void TestsPrinter::verboseFunctionCall(const Tests::MethodDescription &methodDescription,
                                       const Tests::MethodTestCase &testCase,
                                       ErrorMode errorMode) {
    std::string baseReturnType = types::TypesHandler::cBoolToCpp(methodDescription.returnType.baseType());
    types::Type expectedType = typesHandler->getReturnTypeToCheck(methodDescription.returnType);
    if (methodDescription.returnType.maybeReturnArray()) {
        expectedType = methodDescription.returnType.arrayClone(types::PointerUsage::RETURN);
    }
    strComment("Trigger the function");
    std::string methodCall = constrVisitorFunctionCall(methodDescription, testCase, true, errorMode);
    if (!types::TypesHandler::skipTypeInReturn(methodDescription.returnType) && !testCase.isError()) {
        size_t returnPointersCount = 0;
        if (testCase.returnValue.view->getEntryValue(nullptr) == PrinterUtils::C_NULL) {
            returnPointersCount = methodDescription.returnType.countReturnPointers(true);
        }
        auto type = Printer::getConstQualifier(expectedType) + expectedType.usedType();
        strDeclareVar(type, PrinterUtils::ACTUAL, methodCall, std::nullopt, true, returnPointersCount);
    } else {
        ss << LINE_INDENT() << methodCall << SCNL;
    }
}

void TestsPrinter::verboseAsserts(const Tests::MethodDescription &methodDescription,
                                  const Tests::MethodTestCase &testCase,
                                  const std::optional<LineInfo::PredicateInfo>& predicateInfo) {
    strComment("Check results");
    if (types::TypesHandler::isVoid(methodDescription.returnType)) {
        strComment("No check results for void function");
    } else if (types::TypesHandler::isPointerToFunction(methodDescription.returnType) ||
               types::TypesHandler::isArrayOfPointersToFunction(methodDescription.returnType)) {
        strComment("No check results for function returning pointer to function");
    } else if (methodDescription.isConstructor()) {
        strComment("No check results for constructor in current version");
    } else {
        auto visitor = visitor::VerboseAssertsReturnValueVisitor(typesHandler, this, predicateInfo);
        visitor.visit(methodDescription, testCase);
    }

    if (!methodDescription.globalParams.empty()) {
        ss << printer::NL;
        strComment("Check global variables");
        globalParamsAsserts(methodDescription, testCase);
    }

    if (methodDescription.isClassMethod()) {
        ss << printer::NL;
        strComment("Check class fields mutation");
        classAsserts(methodDescription, testCase);
    }

    if (!testCase.paramPostValues.empty()) {
        ss << printer::NL;
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
        std::string expectedName = PrinterUtils::getExpectedVarName(param.name);
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
            std::string expectedName = PrinterUtils::getExpectedVarName(param.name);
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
        std::string expectedName = PrinterUtils::getExpectedVarName(param.name);
        auto expectedType = typesHandler->getReturnTypeToCheck(param.type);
        Tests::MethodParam expectedParam{expectedType, expectedName, param.alignment};
        parameterVisitor.visit(expectedParam.type, expectedParam.name, value.view.get(), std::nullopt);
        assertsVisitor.visitGlobal(param, param.name);
    }
}

void TestsPrinter::printPointerParameter(const Tests::MethodDescription &methodDescription,
                                         const Tests::MethodTestCase &testCase,
                                         int param_num) {
    const auto &param = methodDescription.params[param_num];
    const auto &value = testCase.paramValues[param_num];
    if (types::TypesHandler::isArrayOfPointersToFunction(param.type)) {
        auto type = getTypedefFunctionPointer(methodDescription.name, param.name, false);
        std::string stubName = StubsUtils::getFunctionPointerStubName(
                methodDescription.getClassTypeName(), methodDescription.name, param.name, false);
        strDeclareArrayOfFunctionPointerVar(type, param.name, stubName);
    } else if (types::TypesHandler::isCStringType(param.type)) {
        strDeclareArrayVar(param.type, param.name, types::PointerUsage::PARAMETER,
                           value.view->getEntryValue(this), param.alignment);
    } else if (!param.type.isFilePointer() &&
               (param.type.isObjectPointer() || param.type.isArray())) {
        auto arrayType = types::TypesHandler::isVoid(param.type.baseTypeObj())
                             ? types::Type::minimalScalarPointerType(
                                   param.type.arraysSizes(types::PointerUsage::PARAMETER).size())
                             : param.type;
        if (param.type.maybeJustPointer()) {
            strDeclareVar(arrayType.baseType(), param.name, value.view->getEntryValue(this),
                          param.alignment);
        } else {
            auto paramName =
                param.type.isTwoDimensionalPointer() ? param.underscoredName() : param.name;
            strDeclareArrayVar(arrayType, paramName, types::PointerUsage::PARAMETER,
                               value.view->getEntryValue(this), param.alignment, true);
        }
    }
    if (param.type.isTwoDimensionalPointer()) {
        gen2DPointer(param, true);
    }
}

void TestsPrinter::parametrizedAsserts(const Tests::MethodDescription &methodDescription,
                                       const Tests::MethodTestCase &testCase,
                                       const std::optional<LineInfo::PredicateInfo>& predicateInfo,
                                       ErrorMode errorMode) {
    auto visitor = visitor::ParametrizedAssertsVisitor(typesHandler, this, predicateInfo, testCase.isError());
    if (!methodDescription.isConstructor()) {
        visitor.visit(methodDescription, testCase, errorMode);
    }
    markTestedFunctionCallIfNeed(methodDescription.name, testCase);
    if (!testCase.isError()) {
        globalParamsAsserts(methodDescription, testCase);
        classAsserts(methodDescription, testCase);
        changeableParamsAsserts(methodDescription, testCase);
    } else {
        printFailAssertion(errorMode);
    }
}

void TestsPrinter::markTestedFunctionCallIfNeed(const std::string &name,
                                                const Tests::MethodTestCase &testCase) {
    if (testCase.errorDescriptors.empty()) {
        // cannot generate stack for error
        return;
    }
    ss << sarif::PREFIX_FOR_JSON_PATH << name << "," << testCase.testIndex << printer::NL;
}

std::vector<std::string>
TestsPrinter::methodParametersListParametrized(const Tests::MethodDescription &methodDescription,
                                               const Tests::MethodTestCase &testCase) {
    std::vector<std::string> args;
    for (size_t i = 0; i < methodDescription.params.size(); ++i) {
        Tests::MethodParam const &param = methodDescription.params[i];
        if (param.type.isTwoDimensionalPointer() &&
            types::TypesHandler::isVoid(param.type.baseTypeObj())) {
            std::string qualifier = Printer::getConstQualifier(param.type);
            std::string arg = StringUtils::stringFormat("(%svoid **) %s", qualifier, param.name);
            args.push_back(arg);
        } else if (param.type.isObjectPointer() || param.type.isArray()) {
            std::string maybeAmpersand =
                param.type.maybeJustPointer() && !param.type.isFilePointer() ? "&" : "";
            args.push_back(maybeAmpersand + param.name);
        } else if (param.type.isLValueReference()) {
            args.push_back(param.name);
        } else if (!testCase.paramValues[i].lazyValues.empty()) {
            args.push_back(param.name);
        } else {
            args.push_back(testCase.paramValues[i].view->getEntryValue(this));
        }
    }
    return args;
}

std::vector<std::string>
TestsPrinter::methodParametersListVerbose(const Tests::MethodDescription &methodDescription,
                                          const Tests::MethodTestCase &testCase) {
    std::vector<std::string> args;
    for (const auto &param : methodDescription.params) {
        args.push_back(param.getFunctionParamDecl());
    }
    return args;
}

std::string TestsPrinter::constrVisitorFunctionCall(const Tests::MethodDescription &methodDescription,
                                                    const Tests::MethodTestCase &testCase,
                                                    bool verboseMode,
                                                    ErrorMode errorMode) {
    std::vector<std::string> methodArgs =
        verboseMode ? methodParametersListVerbose(methodDescription, testCase)
                    : methodParametersListParametrized(methodDescription, testCase);

    std::optional<types::Type> castType;
    if (types::TypesHandler::skipTypeInReturn(methodDescription.returnType.baseTypeObj()) &&
        methodDescription.returnType.isObjectPointer()) {
        castType = types::Type::minimalScalarPointerType();
    }
    auto classObjName = methodDescription.getClassName();
    size_t returnPointersCount = 0;
    if (testCase.returnValue.view && testCase.returnValue.view->getEntryValue(nullptr) != PrinterUtils::C_NULL) {
        returnPointersCount = methodDescription.returnType.countReturnPointers(true);
    }
    std::string functionCall = constrFunctionCall(methodDescription.callName, methodArgs, "", classObjName,
                                                  false, returnPointersCount, castType);
    if (methodDescription.isMoveConstructor()) {
        functionCall = "std::move(" + functionCall + ")";
    }
    switch (errorMode) {
        case ErrorMode::PASSING:
            if (testCase.errorInfo.errorType == ErrorType::EXCEPTION_THROWN) {
                functionCall = "EXPECT_ANY_THROW(" + functionCall + ")";
            } else if (testCase.isError()) {
                functionCall = "ASSERT_DEATH(" + functionCall + ", \".*\")";
            }
            break;
        case ErrorMode::PASSING_IN_TARGET_ONLY:
            // TODO: generate EXPECT_ANY_THROW and ASSERT_DEATH only if runtime error was in target function
        default:
            break;
    }
    return functionCall;
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

void TestsPrinter::printFailAssertion(ErrorMode errorMode) {
    switch (errorMode) {
        case ErrorMode::FAILING:
            ss << printer::NL;
            ss << LINE_INDENT()
               << "FAIL() << \"Unreachable point or the function was supposed to fail, but \"\n"
               << LINE_INDENT() << LINE_INDENT()
               << "\"actually completed successfully. See the SARIF report for details.\"";
            ss << SCNL;
            break;
        case ErrorMode::PASSING_IN_TARGET_ONLY:
            //TODO add passing scenario
        case ErrorMode::PASSING:
        default:
            break;
    }
}

std::string printer::MultiLinePrinter::print(TestsPrinter *printer,
                                             const tests::StructValueView *view) {
    auto subViews = view->getSubViews();
    std::stringstream structuredValuesWithPrefixes;

    structuredValuesWithPrefixes << (view->isAnonymous() ? "/* { */" : "{") << printer::NL;
    ++printer->tabsDepth;

    const size_t fieldIndexToInitUnion = view->getFieldIndexToInitUnion();
    const bool isStruct = view->getStructInfo().subType == types::SubType::Struct;

    size_t i = 0;
    for (const auto &sview : subViews) {
        if (i != 0) {
            if (isStruct)
                structuredValuesWithPrefixes << ",";
            structuredValuesWithPrefixes << printer::NL;
        }

        bool printInComment = !(isStruct || fieldIndexToInitUnion == i);
        if (printInComment) {
            ++printer->commentDepth;
        }
        structuredValuesWithPrefixes << printer->LINE_INDENT()
                                     << view->getFieldPrefix(i)
                                     << sview->getEntryValue(printer);
        if (printInComment) {
            --printer->commentDepth;
        }

        ++i;
    }

    --printer->tabsDepth;
    structuredValuesWithPrefixes << printer::NL
                                 << printer->LINE_INDENT()
                                 << (view->isAnonymous() ? "/* } */" : "}");

    return structuredValuesWithPrefixes.str();
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
