/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_BASETEST_H
#define UNITTESTBOT_BASETEST_H

#include <gtest/gtest.h>

#include "Server.h"
#include "utils/CompilationUtils.h"
#include "TestUtils.h"

#include "utils/path/FileSystemPath.h"

using CompilationUtils::CompilerName;

class BaseTest : public testing::Test {
protected:
    explicit BaseTest(const string& suiteName);

    const bool TESTMODE = true;
    Server server = Server(TESTMODE); // testmode on
    fs::path baseSuitePath = fs::current_path().parent_path() / testUtils::getRelativeTestSuitePath("");
    string suiteName;
    fs::path suitePath;

    CompilerName compilerName = CompilerName::CLANG;
    string buildDirRelativePath;
    fs::path buildPath;
    std::vector<fs::path> srcPaths;

    const std::string projectName = "unittest-project";
    std::string testDirName = "test"; // name of folder inside server, not in suite
    std::unique_ptr<ServerContext> ctx;
    std::unique_ptr<TestsWriter> writer;

    void setSuite(const string &suite);

    void setCompiler(CompilationUtils::CompilerName name);

    void setBuildDirectory(const string &buildDirectoryName);

    fs::path getTestFilePath(const string& fileName);

    void clearEnv();

    void clearTestDirectory();

    void clearDirectory(const fs::path &pathToDirectory);

    fs::path getTestDirectory();

    fs::path getStubsDirectory();

    fs::path getPathToGeneratedTestFileByTestedFile(const string &fileName);

    fs::path getPathToGeneratedTestHeaderFileByTestedFile(const string &fileName);
};

#endif //UNITTESTBOT_BASETEST_H
