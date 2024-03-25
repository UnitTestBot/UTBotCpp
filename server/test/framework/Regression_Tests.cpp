#include "gtest/gtest.h"

#include "BaseTest.h"
#include "KleeGenerator.h"
#include "Server.h"
#include "TestUtils.h"

#include "utils/path/FileSystemPath.h"
#include <functional>

namespace {
    using grpc::Channel;
    using grpc::ClientContext;
    using testsgen::TestsGenService;
    using testsgen::TestsResponse;
    using testUtils::checkTestCasePredicates;
    using testUtils::createLineRequest;

    class Regression_Test : public BaseTest {
    protected:
        Regression_Test() : BaseTest("regression") {
        }

        void SetUp() override {
            clearEnv(CompilationUtils::CompilerName::CLANG);
        }

        std::pair<FunctionTestGen, Status>
        createTestForFunction(const fs::path &pathToFile, int lineNum, bool verbose = true) {
            auto lineRequest = testUtils::createLineRequest(projectName, suitePath, buildDirRelativePath,
                                                            srcPaths, pathToFile, lineNum, "",
                                                            pathToFile, false, verbose, 0);
            auto request = GrpcUtils::createFunctionRequest(std::move(lineRequest));
            auto testGen = FunctionTestGen(*request, writer.get(), TESTMODE);

            Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
            return {testGen, status};
        }

        std::pair<FolderTestGen, Status>
        createTestForFolder(const fs::path &pathToFolder, bool useStubs = true, bool verbose = true) {
            auto folderRequest = testUtils::createProjectRequest(projectName, suitePath, buildDirRelativePath,
                                                                 srcPaths, "", GrpcUtils::UTBOT_AUTO_TARGET_PATH, useStubs,
                                                                 verbose, 0);
            auto request = GrpcUtils::createFolderRequest(std::move(folderRequest), pathToFolder);
            auto testGen = FolderTestGen(*request, writer.get(), TESTMODE);

            Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
            return {testGen, status};
        }
    };

    // uint_32t parameters/return values and call external printf
    TEST_F(Regression_Test, SAT_372_Printf_Symbolic_Parameter) {
        fs::path helloworld_c = getTestFilePath("helloworld.c");

        auto [testGen, status] = createTestForFunction(helloworld_c, 10);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(helloworld_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>({ [](const tests::Tests::MethodTestCase &testCase) {
                int ret = stoi(testCase.returnValue.view->getEntryValue(nullptr));
                int param = stoi(testCase.paramValues[0].view->getEntryValue(nullptr));
                return ret == param + 1;
            } }),
            "helloworld");
    }

    TEST_F(Regression_Test, Null_Return) {
        fs::path source = getTestFilePath("SAT-752.c");

        for (bool verbose : { false, true }) {
            auto [testGen, status] = createTestForFunction(source, 7, verbose);

            ASSERT_TRUE(status.ok()) << status.error_message();

            checkTestCasePredicates(
                testGen.tests.at(source).methods.begin().value().testCases,
                std::vector<TestCasePredicate>({ [](const tests::Tests::MethodTestCase &testCase) {
                    return testCase.returnValue.view->getEntryValue(nullptr) == PrinterUtils::C_NULL;
                } }),
                "byword");
        }
    }

    // struct definition, declaration, usage in separate files
    TEST_F(Regression_Test, Incomplete_Array_Type) {
        fs::path folderPath = suitePath / "SAT-760";
        auto projectRequest = testUtils::createProjectRequest(
            projectName, suitePath, buildDirRelativePath, { suitePath, folderPath }, "", "SAT-760");
        auto request = GrpcUtils::createFolderRequest(std::move(projectRequest), folderPath);
        auto testGen = FolderTestGen(*request, writer.get(), TESTMODE);

        fs::path source1 = folderPath / "SAT-760_1.c";
        fs::path source2 = folderPath / "SAT-760_2.c";
        Tests tests1 = testGen.tests.at(source1);
        Tests tests2 = testGen.tests.at(source2);
        testGen.tests.clear();
        testGen.tests[source1] = tests1;
        testGen.tests[source2] = tests2;
        // Reorder files in order to parse them in the fixed manner.

        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();
        {
            checkTestCasePredicates(
                testGen.tests.at(source1).methods.begin().value().testCases,
                std::vector<TestCasePredicate>({ [](const tests::Tests::MethodTestCase &testCase) {
                  EXPECT_EQ(1, testCase.globalPreValues.size());
                  EXPECT_EQ(1, testCase.globalPostValues.size());
                  return true;
                } }),
                "write");
        }
        {
            checkTestCasePredicates(
                testGen.tests.at(source2).methods.begin().value().testCases,
                std::vector<TestCasePredicate>({ [](const tests::Tests::MethodTestCase &testCase) {
                    EXPECT_EQ(0, testCase.globalPreValues.size());
                    EXPECT_EQ(0, testCase.globalPostValues.size());
                    return true;
                } }),
                "write");
        }
    }

    // array type needs an explicit size or an initializer
    TEST_F(Regression_Test, Global_Char_Array) {
        fs::path source = getTestFilePath("SAT-766.c");

        auto [testGen, status] = createTestForFunction(source, 4);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(source).methods.begin().value().testCases,
            std::vector<TestCasePredicate>({ [](const tests::Tests::MethodTestCase &testCase) {
              EXPECT_EQ(1, testCase.globalPreValues.size());
              EXPECT_EQ(1, testCase.globalPostValues.size());
              return !testCase.isError();
            } }),
            "first");
    }

    // Index array into array of struct
    TEST_F(Regression_Test, Index_Out_Of_Bounds) {
        fs::path source = getTestFilePath("SAT-767.c");
        auto [testGen, status] = createTestForFunction(source, 8);

        ASSERT_TRUE(status.ok()) << status.error_message();


        checkTestCasePredicates(
            testGen.tests.at(source).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                { [](const tests::Tests::MethodTestCase &testCase) {
                  EXPECT_EQ(1, testCase.globalPreValues.size());
                  EXPECT_EQ(1, testCase.globalPostValues.size());
                  return !testCase.isError();
                } }),
            "first");
    }

    // null pointer
    TEST_F(Regression_Test, Global_Array_Of_Pointers) {
        fs::path source = getTestFilePath("SAT-777.c");
        auto [testGen, status] = createTestForFunction(source, 5);

        ASSERT_TRUE(status.ok()) << status.error_message();


        checkTestCasePredicates(
            testGen.tests.at(source).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                { [](const tests::Tests::MethodTestCase &testCase) {
                  EXPECT_EQ(1, testCase.globalPreValues.size());
                  EXPECT_EQ(1, testCase.globalPostValues.size());
                  return !testCase.isError();
                } }),
            "set_file_list");
    }

    TEST_F(Regression_Test, Return_Pointer_Argument_GNU_90) {
        fs::path source = getTestFilePath("PR120.c");
        auto [testGen, status] = createTestForFunction(source, 2);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(source).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                { [](const tests::Tests::MethodTestCase &testCase) {
                    return !testCase.isError();
                } }),
            "ret");
    }

    TEST_F(Regression_Test, VaList_In_Function_Pointer_Type) {
        fs::path source = getTestFilePath("PR123.c");
        auto [testGen, status] = createTestForFunction(source, 7);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(source).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                { [](const tests::Tests::MethodTestCase &testCase) {
                    return !testCase.isError();
                } }),
            "libbpf_set_print");
    }

    TEST_F(Regression_Test, Unused_Function_Pointer_Parameter) {
        fs::path source = getTestFilePath("PR153.c");
        auto [testGen, status] = createTestForFunction(source, 5);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(source).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                { [](const tests::Tests::MethodTestCase &testCase) {
                    return testCase.isError();
                } }),
            "unused");
    }

    TEST_F(Regression_Test, No_Such_Type_Exception) {
        fs::path folderPath = suitePath / "PR-200";
        fs::path source = folderPath / "PR-200.c";
        auto [testGen, status] = createTestForFunction(source, 10);

        ASSERT_TRUE(status.ok()) << status.error_message();
    }

    TEST_F(Regression_Test, Hash_Of_String) {
        fs::path source = getTestFilePath("GH215.c");
        auto [testGen, status] = createTestForFunction(source, 2);

        ASSERT_TRUE(status.ok()) << status.error_message();

        auto predicate = [](const tests::Tests::MethodTestCase &testCase) {
            auto s = testCase.paramValues[0].view->getEntryValue(nullptr);
            s = s.substr(1, s.length() - 2);
            size_t i = 0;
            unsigned int sum = 0;
            //Hash count of word`s letters
            while (s.substr(i, 4) != "'\\0'"){
                sum += s[i + 1];
                i += 5;// Go to next letter
            }

            auto actual = testCase.returnValue.view->getEntryValue(nullptr);
            auto expected = std::to_string(sum);
            return actual == expected;
        };

        checkTestCasePredicates(
            testGen.tests.at(source).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                { [&predicate](const tests::Tests::MethodTestCase &testCase) {
                     // empty string
                     return testCase.paramValues[0].view->getEntryValue(nullptr).substr(2, 2) == "\\0" &&
                            predicate(testCase);
                 },
                  [&predicate](const tests::Tests::MethodTestCase &testCase) {
                      // non-empty string
                      return testCase.paramValues[0].view->getEntryValue(nullptr).substr(2, 2) != "\\0" &&
                             predicate(testCase);
                  } }),
            "hash");
    }

    TEST_F(Regression_Test, Export_Empty) {
        fs::path source = getTestFilePath("issue-276.c");
        auto [testGen, status] = createTestForFunction(source, 2);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(source).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        { [](const tests::Tests::MethodTestCase &testCase) {
                            return testCase.returnValue.view->getEntryValue(nullptr) == "0";
                        } }),
                "f1");
    }

    TEST_F(Regression_Test, Export_Empty_String) {
        fs::path source = getTestFilePath("issue-276.c");
        auto [testGen, status] = createTestForFunction(source, 6);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(source).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        { [](const tests::Tests::MethodTestCase &testCase) {
                            return testCase.returnValue.view->getEntryValue(nullptr) == "'\\0'";
                        } }),
                "f2");
    }

    TEST_F(Regression_Test, Export_Int) {
        fs::path source = getTestFilePath("issue-276.c");
        auto [testGen, status] = createTestForFunction(source, 10);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(source).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        { [](const tests::Tests::MethodTestCase &testCase) {
                            return testCase.returnValue.view->getEntryValue(nullptr) == "4";
                        } }),
                "f3");
    }

    TEST_F(Regression_Test, Export_String_Int) {
        fs::path source = getTestFilePath("issue-276.c");
        auto [testGen, status] = createTestForFunction(source, 14);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(source).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        { [](const tests::Tests::MethodTestCase &testCase) {
                            return testCase.returnValue.view->getEntryValue(nullptr) == "'4'";
                        } }),
                "f4");
    }

    static bool checkAlignmentInNode(const tests::Tests::TestCaseParamValue &param) {
        static const size_t HEX = 16;
        static const size_t NODE_ALIGNMENT = 8;

        const auto stringPointer = param.view->getSubViews().at(1)->getEntryValue(nullptr);
        const size_t address = strtoull(stringPointer.c_str(), nullptr, HEX);
        bool result = (address % NODE_ALIGNMENT == 0);

        for (const auto &innerLazyValueParam : param.lazyValues) {
            result &= checkAlignmentInNode(innerLazyValueParam);
        }
        return result;
    }

    TEST_F(Regression_Test, DISABLED_Pointers_Alignment) {
        fs::path source = getTestFilePath("issue-195.c");
        auto [testGen, status] = createTestForFunction(source, 8);

        ASSERT_TRUE(status.ok()) << status.error_message();

        for (const tests::Tests::MethodTestCase &testCase :
             testGen.tests.at(source).methods.begin().value().testCases) {
            for (const auto &paramValue : testCase.paramValues) {
                EXPECT_TRUE(checkAlignmentInNode(paramValue));
            }
        }
    }

    TEST_F(Regression_Test, Generate_Folder) {
        fs::path folderPath = getTestFilePath("ISSUE-140");
        auto [testGen, status] = createTestForFolder(folderPath, true, true);
        ASSERT_TRUE(status.ok()) << status.error_message();
        testUtils::checkMinNumberOfTests(testGen.tests, 1);
    }

    TEST_F(Regression_Test, Extern_Variables) {
        fs::path source = getTestFilePath("issue-514.c");
        auto [testGen, status] = createTestForFunction(source, 5);
        ASSERT_TRUE(status.ok()) << status.error_message();
        testUtils::checkMinNumberOfTests(testGen.tests, 2);
    }

    TEST_F(Regression_Test, Unnecessary_Enum_Specifier) {
        fs::path source = getTestFilePath("issue-600.c");
        auto [testGen, status] = createTestForFunction(source, 14);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(source).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {[](const tests::Tests::MethodTestCase &testCase) {
                        return testCase.returnValue.view->getEntryValue(nullptr) == "-2";
                    },
                    [](const tests::Tests::MethodTestCase &testCase) {
                        return testCase.returnValue.view->getEntryValue(nullptr) == "-1";
                    },
                    [](const tests::Tests::MethodTestCase &testCase) {
                        return testCase.returnValue.view->getEntryValue(nullptr) == "0";
                    },
                    [](const tests::Tests::MethodTestCase &testCase) {
                        return testCase.returnValue.view->getEntryValue(nullptr) == "1";
                    }
                }),
            "isCorrectPointerStruct"
        );
    }
}
