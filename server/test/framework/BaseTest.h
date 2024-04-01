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
    explicit BaseTest(const std::string &suiteName);

    const bool TESTMODE = true;
    Server server = Server(TESTMODE); // testmode on
    fs::path baseSuitePath = fs::current_path().parent_path() / testUtils::getRelativeTestSuitePath("");
    std::string suiteName;
    fs::path suitePath;

    CompilerName compilerName = CompilerName::CLANG;
    std::string buildDirRelPath;
    std::string testsDirRelPath = Paths::UTBOT_TESTS;
    std::string reportsDirRelPath = Paths::UTBOT_REPORT;
    std::string clientProjectPath = Paths::UTBOT_ITF;
    fs::path buildPath;
    std::vector<fs::path> srcPaths;

    const std::string projectName = "unittest-project";
    std::string testDirName = "test"; // name of folder inside server, not in suite
    std::unique_ptr<ServerContext> ctx;
    std::unique_ptr<TestsWriter> writer;

    void setSuite(const std::string &suite);

    void setCompiler(CompilationUtils::CompilerName name);

    void setBuildDirectory(const std::string &buildDirectoryName);

    fs::path getTestFilePath(const std::string &fileName);

    void clearTestDirectory();

    void clearDirectory(const fs::path &pathToDirectory);

    fs::path getTestDirectory();

    fs::path getStubsDirectory();

    fs::path getPathToGeneratedTestFileByTestedFile(const std::string &fileName);

    fs::path getPathToGeneratedTestHeaderFileByTestedFile(const std::string &fileName);

    void clearEnv(const CompilationUtils::CompilerName &compiler);
};

#endif //UNITTESTBOT_BASETEST_H
