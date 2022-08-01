#include "gtest/gtest.h"

#include "BaseTest.h"

namespace {
    class Library_Test : public BaseTest {
    protected:
        Library_Test() : BaseTest("library-project") {
        }

        fs::path test = getTestFilePath("src");
        fs::path sum = getTestFilePath("lib");
        fs::path test_c = test / "test.c";

        void SetUp() override {
            clearEnv(CompilationUtils::CompilerName::CLANG);
            srcPaths = { suitePath, test, sum };
        }

        std::pair<FunctionTestGen, Status>
        createTestForFunction(const fs::path &pathToFile, int lineNum, int kleeTimeout = 60) {
            auto lineRequest = testUtils::createLineRequest(projectName, suitePath, buildDirRelativePath,
                                                            srcPaths, pathToFile, lineNum, std::nullopt, true, false, kleeTimeout);
            auto request = GrpcUtils::createFunctionRequest(std::move(lineRequest));
            auto testGen = FunctionTestGen(*request, writer.get(), TESTMODE);
            testGen.setTargetForSource(pathToFile);
            Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
            return { testGen, status };
        }
    };

    TEST_F(Library_Test, sum) {
        auto [testGen, status] = createTestForFunction(test_c, 3);

        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkTestCasePredicates(
            testGen.tests.at(test_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>({
                [](const tests::Tests::MethodTestCase &testCase) {
                     return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1;
                 },
                [](const tests::Tests::MethodTestCase &testCase) {
                      return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -1;
                }
            })
        );

        auto [testGen2, status2] = createTestForFunction(test_c, 3);

        ASSERT_TRUE(status2.ok()) << status2.error_message();

        testUtils::checkTestCasePredicates(
            testGen2.tests.at(test_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>({
                [](const tests::Tests::MethodTestCase &testCase) {
                    return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1;
                },
                [](const tests::Tests::MethodTestCase &testCase) {
                    return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -1;
                }
            })
        );
    }
}
