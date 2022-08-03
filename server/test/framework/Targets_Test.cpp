#include "BaseTest.h"
#include "building/BuildDatabase.h"

class TargetsTest : public BaseTest {
protected:
    TargetsTest() : BaseTest("targets") {
    }

    void SetUp() override {
        clearEnv(CompilationUtils::CompilerName::CLANG);
    }

    fs::path parse_c = getTestFilePath("parse.c");
    fs::path get_10_c = getTestFilePath("get_10.c");
    fs::path get_20x_c = getTestFilePath("get_20x.c");
    fs::path shared_c = getTestFilePath("shared.c");
    fs::path get_val_main_c = getTestFilePath("get_val_main.c");
    fs::path get_val_main_2_c = getTestFilePath("get_val_main_2.c");
};

using namespace testUtils;

TEST_F(TargetsTest, Valid_Target_Test_ls) {
    auto projectRequest =
            createProjectRequest(projectName, suitePath, buildDirRelativePath, srcPaths, "ls");
    auto request = GrpcUtils::createFileRequest(std::move(projectRequest), parse_c);
    auto testGen = FileTestGen(*request, writer.get(), TESTMODE);
//    fs::path ls = getTargetPathByName(*testGen.buildDatabase, "ls");
//    testGen.setTargetPath(ls);

    Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
    ASSERT_TRUE(status.ok()) << status.error_message();

    checkMinNumberOfTestsInFile(testGen, parse_c, 1);
    checkTestCasePredicates(
            testGen.tests.at(parse_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>({[](const tests::Tests::MethodTestCase &testCase) {
                std::string ret = testCase.returnValue.view->getEntryValue(nullptr);
                return ret == "\'l\'";
            }}),
            "parse");
}

TEST_F(TargetsTest, Valid_Target_Test_cat) {
    auto projectRequest =
            createProjectRequest(projectName, suitePath, buildDirRelativePath, srcPaths, "cat");
    auto request = GrpcUtils::createFileRequest(std::move(projectRequest), parse_c);
    auto testGen = FileTestGen(*request, writer.get(), TESTMODE);
//    fs::path cat = getTargetPathByName(*testGen.buildDatabase, "cat");
//    testGen.setTargetPath(cat);

    Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
    ASSERT_TRUE(status.ok()) << status.error_message();

    checkMinNumberOfTestsInFile(testGen, parse_c, 1);
    checkTestCasePredicates(
            testGen.tests.at(parse_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>({[](const tests::Tests::MethodTestCase &testCase) {
                std::string ret = testCase.returnValue.view->getEntryValue(nullptr);
                return ret == "\'c\'";
            }}),
            "parse");
}

TEST_F(TargetsTest, Valid_Target_Test_dummy) {
    auto projectRequest = createProjectRequest(projectName, suitePath, buildDirRelativePath, srcPaths, "dummy");
    auto request = GrpcUtils::createFileRequest(std::move(projectRequest), parse_c);
    auto testGen = FileTestGen(*request, writer.get(), TESTMODE);
//    fs::path dummy = getTargetPathByName(*testGen.buildDatabase, "dummy");
//    testGen.setTargetPath(dummy);

    Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
    ASSERT_TRUE(status.ok()) << status.error_message();

    int numberOfTests = testUtils::getNumberOfTests(testGen.tests);
    EXPECT_EQ(0, numberOfTests);
}

TEST_F(TargetsTest, Valid_Target_Test_parse) {
    auto projectRequest = createProjectRequest(projectName, suitePath, buildDirRelativePath, srcPaths);
    auto request = GrpcUtils::createFileRequest(std::move(projectRequest), parse_c);
    auto testGen = FileTestGen(*request, writer.get(), TESTMODE);
//    fs::path autoTarget = GrpcUtils::UTBOT_AUTO_TARGET_PATH;
//    testGen.setTargetPath(autoTarget);

    Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
    ASSERT_TRUE(status.ok()) << status.error_message();
    checkMinNumberOfTestsInFile(testGen, parse_c, 1);
    checkTestCasePredicates(
            testGen.tests.at(parse_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>({[](const tests::Tests::MethodTestCase &testCase) {
                std::string ret = testCase.returnValue.view->getEntryValue(nullptr);
                return ret == "\'c\'" || ret == "\'l\'";
            }}),
            "parse");
}

TEST_F(TargetsTest, Valid_Target_Test_get_10) {
    std::unique_ptr<ProjectRequest> projectRequest = createProjectRequest(
            projectName, suitePath, buildDirRelativePath, srcPaths, "get_10", false, false, 15);

    auto testGen = ProjectTestGen(*projectRequest.get(), writer.get(), TESTMODE, true);
//    fs::path get_10 = getTargetPathByName(*testGen.buildDatabase, "get_10");
//    testGen.setTargetPath(get_10);

    Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
    ASSERT_TRUE(status.ok()) << status.error_message();

    checkMinNumberOfTestsInFile(testGen, get_10_c, 1);
    checkTestCasePredicates(
            testGen.tests.at(get_10_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>({[](const tests::Tests::MethodTestCase &testCase) {
                return testCase.returnValue.view->getEntryValue(nullptr) == "10";
            }}),
            "get_any_val");

    checkMinNumberOfTestsInFile(testGen, shared_c, 1);
    checkTestCasePredicates(
            testGen.tests.at(shared_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>({[](const tests::Tests::MethodTestCase &testCase) {
                return std::stoi(testCase.returnValue.view->getEntryValue(nullptr)) ==
                       (10 + std::stoi(testCase.paramValues.front().view->getEntryValue(nullptr)));
            }}),
            "add_val");

    checkMinNumberOfTestsInFile(testGen, get_val_main_c, 2);
    checkTestCasePredicates(
            testGen.tests.at(get_val_main_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>({[](const tests::Tests::MethodTestCase &testCase) {
                return std::stoi(testCase.returnValue.view->getEntryValue(nullptr)) ==
                       (10 + 0);
            }}),
            "get_res");

    checkNumberOfTestsInFile(testGen, parse_c, 0);
    checkNumberOfTestsInFile(testGen, get_20x_c, 0);
    checkNumberOfTestsInFile(testGen, get_val_main_2_c, 0);
}

TEST_F(TargetsTest, Valid_Target_Test_get_20) {
    std::unique_ptr<ProjectRequest> projectRequest = createProjectRequest(
            projectName, suitePath, buildDirRelativePath, srcPaths, "get_20", false, false, 15);

    auto testGen = ProjectTestGen(*projectRequest.get(), writer.get(), TESTMODE, true);
//    fs::path get_20 = getTargetPathByName(*testGen.buildDatabase, "get_20");

    Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
    ASSERT_TRUE(status.ok()) << status.error_message();

    checkMinNumberOfTestsInFile(testGen, get_20x_c, 1);
    checkTestCasePredicates(
            testGen.tests.at(get_20x_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>({[](const tests::Tests::MethodTestCase &testCase) {
                return testCase.returnValue.view->getEntryValue(nullptr) == "20";
            }}),
            "get_any_val");

    checkMinNumberOfTestsInFile(testGen, shared_c, 1);
    checkTestCasePredicates(
            testGen.tests.at(shared_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>({[](const tests::Tests::MethodTestCase &testCase) {
                return std::stoi(testCase.returnValue.view->getEntryValue(nullptr)) ==
                       (20 + std::stoi(testCase.paramValues.front().view->getEntryValue(nullptr)));
            }}),
            "add_val");

    checkMinNumberOfTestsInFile(testGen, get_val_main_c, 2);
    checkTestCasePredicates(
            testGen.tests.at(get_val_main_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>({[](const tests::Tests::MethodTestCase &testCase) {
                return std::stoi(testCase.returnValue.view->getEntryValue(nullptr)) ==
                       (20 + 0);
            }}),
            "get_res");

    checkNumberOfTestsInFile(testGen, parse_c, 0);
    checkNumberOfTestsInFile(testGen, get_10_c, 0);
    checkNumberOfTestsInFile(testGen, get_val_main_2_c, 0);
}

TEST_F(TargetsTest, Valid_Target_Test_get_10_2) {
    std::unique_ptr<ProjectRequest> projectRequest = createProjectRequest(
            projectName, suitePath, buildDirRelativePath, srcPaths, "get_10_2", false, false, 15);

    auto testGen = ProjectTestGen(*projectRequest.get(), writer.get(), TESTMODE, true);
//    fs::path get_10_2 = getTargetPathByName(*testGen.buildDatabase, "get_10_2");
//    testGen.setTargetPath(get_10_2);

    Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
    ASSERT_TRUE(status.ok()) << status.error_message();

    checkMinNumberOfTestsInFile(testGen, get_10_c, 1);
    checkTestCasePredicates(
            testGen.tests.at(get_10_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>({[](const tests::Tests::MethodTestCase &testCase) {
                return testCase.returnValue.view->getEntryValue(nullptr) == "10";
            }}),
            "get_any_val");

    checkMinNumberOfTestsInFile(testGen, shared_c, 1);
    checkTestCasePredicates(
            testGen.tests.at(shared_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>({[](const tests::Tests::MethodTestCase &testCase) {
                return std::stoi(testCase.returnValue.view->getEntryValue(nullptr)) ==
                       (10 + std::stoi(testCase.paramValues.front().view->getEntryValue(nullptr)));
            }}),
            "add_val");

    checkMinNumberOfTestsInFile(testGen, get_val_main_2_c, 2);
    checkTestCasePredicates(
            testGen.tests.at(get_val_main_2_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>({[](const tests::Tests::MethodTestCase &testCase) {
                return std::stoi(testCase.returnValue.view->getEntryValue(nullptr)) ==
                       (10 + 5);
            }}),
            "get_res");

    checkNumberOfTestsInFile(testGen, parse_c, 0);
    checkNumberOfTestsInFile(testGen, get_20x_c, 0);
    checkNumberOfTestsInFile(testGen, get_val_main_c, 0);
}

TEST_F(TargetsTest, Valid_Target_Test_libshared) {
    std::unique_ptr<ProjectRequest> projectRequest = createProjectRequest(
            projectName, suitePath, buildDirRelativePath, srcPaths, "libshared_get.so", false, false, 15);

    auto testGen = ProjectTestGen(*projectRequest.get(), writer.get(), TESTMODE, true);
//    fs::path lib_shared = getTargetPathByName(*testGen.buildDatabase, "libshared_get.so");
//    testGen.setTargetPath(lib_shared);

    Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
    ASSERT_TRUE(status.ok()) << status.error_message();

    checkMinNumberOfTestsInFile(testGen, get_20x_c, 1);
    checkTestCasePredicates(
            testGen.tests.at(get_20x_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>({[](const tests::Tests::MethodTestCase &testCase) {
                return testCase.returnValue.view->getEntryValue(nullptr) == "20";
            }}),
            "get_any_val");

    checkMinNumberOfTestsInFile(testGen, shared_c, 1);
    checkTestCasePredicates(
            testGen.tests.at(shared_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>({[](const tests::Tests::MethodTestCase &testCase) {
                return std::stoi(testCase.returnValue.view->getEntryValue(nullptr)) ==
                       (20 + std::stoi(testCase.paramValues.front().view->getEntryValue(nullptr)));
            }}),
            "add_val");

    checkMinNumberOfTestsInFile(testGen, get_val_main_c, 1);
    checkTestCasePredicates(
            testGen.tests.at(get_val_main_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>({[](const tests::Tests::MethodTestCase &testCase) {
                return std::stoi(testCase.returnValue.view->getEntryValue(nullptr)) ==
                       (20 + 0);
            }}),
            "get_res");

    checkNumberOfTestsInFile(testGen, parse_c, 0);
    checkNumberOfTestsInFile(testGen, get_10_c, 0);
    checkNumberOfTestsInFile(testGen, get_val_main_2_c, 0);
}

TEST_F(TargetsTest, Valid_Target_Test_get_libstatic) {
    std::unique_ptr<ProjectRequest> projectRequest = createProjectRequest(
            projectName, suitePath, buildDirRelativePath, srcPaths, "libstatic_get.a", false, false, 15);

    auto testGen = ProjectTestGen(*projectRequest.get(), writer.get(), TESTMODE, true);
//    fs::path lib_static = getTargetPathByName(*testGen.buildDatabase, "libstatic_get.a");
//    testGen.setTargetPath(lib_static);

    Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
    ASSERT_TRUE(status.ok()) << status.error_message();

    checkMinNumberOfTestsInFile(testGen, get_20x_c, 1);
    checkTestCasePredicates(
            testGen.tests.at(get_20x_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>({[](const tests::Tests::MethodTestCase &testCase) {
                return testCase.returnValue.view->getEntryValue(nullptr) == "200";
            }}),
            "get_any_val");

    checkMinNumberOfTestsInFile(testGen, shared_c, 1);
    checkTestCasePredicates(
            testGen.tests.at(shared_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>({[](const tests::Tests::MethodTestCase &testCase) {
                return std::stoi(testCase.returnValue.view->getEntryValue(nullptr)) ==
                       (200 + std::stoi(testCase.paramValues.front().view->getEntryValue(nullptr)));
            }}),
            "add_val");

    checkMinNumberOfTestsInFile(testGen, get_val_main_c, 2);
    checkTestCasePredicates(
            testGen.tests.at(get_val_main_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>({[](const tests::Tests::MethodTestCase &testCase) {
                return std::stoi(testCase.returnValue.view->getEntryValue(nullptr)) ==
                       (200 + 0);
            }}),
            "get_res");

    checkNumberOfTestsInFile(testGen, parse_c, 0);
    checkNumberOfTestsInFile(testGen, get_10_c, 0);
    checkNumberOfTestsInFile(testGen, get_val_main_2_c, 0);
}

