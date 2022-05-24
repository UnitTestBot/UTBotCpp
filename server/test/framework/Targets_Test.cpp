
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "BaseTest.h"
#include "building/BuildDatabase.h"

class TargetsTest : public BaseTest {
protected:
    TargetsTest() : BaseTest("targets") {
    }

    void SetUp() override {
        clearEnv();
    }

    fs::path parse_c = getTestFilePath("parse.c");

    fs::path getTargetPathByName(BuildDatabase const &buildDatabase, fs::path const &fileName) {
        auto rootTargets = buildDatabase.getRootTargets();
        auto it =
            std::find_if(rootTargets.begin(), rootTargets.end(),
                         [&fileName](std::shared_ptr<BuildDatabase::TargetInfo> linkUnitInfo) {
                             return linkUnitInfo->getOutput().filename() == fileName;
                         });
        assert(it != rootTargets.end());
        return it->get()->getOutput();
    }
};

using namespace testUtils;

TEST_F(TargetsTest, Valid_Target_Test_1) {
    {
        auto projectRequest =
            createProjectRequest(projectName, suitePath, buildDirRelativePath, srcPaths);
        auto request = GrpcUtils::createFileRequest(std::move(projectRequest), parse_c);
        auto testGen = FileTestGen(*request, writer.get(), TESTMODE);
        fs::path ls = getTargetPathByName(*testGen.buildDatabase, "ls");
        testGen.setTargetPath(ls);

        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(parse_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>({ [](const tests::Tests::MethodTestCase &testCase) {
                std::string ret = testCase.returnValue.view->getEntryValue();
                return ret == "\'l\'";
            } }),
            "parse");
    }
    {
        auto projectRequest =
            createProjectRequest(projectName, suitePath, buildDirRelativePath, srcPaths);
        auto request = GrpcUtils::createFileRequest(std::move(projectRequest), parse_c);
        auto testGen = FileTestGen(*request, writer.get(), TESTMODE);
        fs::path cat = getTargetPathByName(*testGen.buildDatabase, "cat");
        testGen.setTargetPath(cat);

        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(parse_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>({ [](const tests::Tests::MethodTestCase &testCase) {
                std::string ret = testCase.returnValue.view->getEntryValue();
                return ret == "\'c\'";
            } }),
            "parse");
    }
    {
        auto projectRequest =
            createProjectRequest(projectName, suitePath, buildDirRelativePath, srcPaths);
        auto request = GrpcUtils::createFileRequest(std::move(projectRequest), parse_c);
        auto testGen = FileTestGen(*request, writer.get(), TESTMODE);
        fs::path dummy = getTargetPathByName(*testGen.buildDatabase, "dummy");
        testGen.setTargetPath(dummy);

        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_FALSE(status.ok());

        int numberOfTests = testUtils::getNumberOfTests(testGen.tests);
        EXPECT_EQ(0, numberOfTests);
    }
    {
        auto projectRequest =
            createProjectRequest(projectName, suitePath, buildDirRelativePath, srcPaths);
        auto request = GrpcUtils::createFileRequest(std::move(projectRequest), parse_c);
        auto testGen = FileTestGen(*request, writer.get(), TESTMODE);
        fs::path autoTarget = GrpcUtils::UTBOT_AUTO_TARGET_PATH;
        testGen.setTargetPath(autoTarget);

        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok());
        checkTestCasePredicates(
                testGen.tests.at(parse_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>({[](const tests::Tests::MethodTestCase &testCase) {
                    std::string ret = testCase.returnValue.view->getEntryValue();
                    return ret == "\'c\'" || ret == "\'l\'";
                }}),
                "parse");
    }
}