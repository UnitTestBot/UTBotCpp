/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "BaseTest.h"

#include "utils/FileSystemUtils.h"
#include "utils/ServerUtils.h"

BaseTest::BaseTest(const string &suiteName) {
    setSuite(suiteName);
}

void BaseTest::setSuite(const string &suite) {
    suiteName = suite;
    suitePath = baseSuitePath / suiteName;
    srcPaths = { suitePath, suitePath / "inner" };
    setCompiler(compilerName);
}

void BaseTest::setCompiler(CompilationUtils::CompilerName name) {
    compilerName = name;
    setBuildDirectory(getBuildDirectoryName(compilerName));
}

void BaseTest::setBuildDirectory(const string &buildDirectoryName) {
    buildDirRelativePath = buildDirectoryName;
    buildPath = suitePath / buildDirRelativePath;
}

fs::path BaseTest::getTestFilePath(const string &fileName) {
    return suitePath / fileName;
}

void BaseTest::clearEnv() {
    compilerName = CompilerName::CLANG;
    setSuite(suiteName);

    ctx = std::make_unique<ServerContext>();
    writer = std::make_unique<ServerTestsWriter>(nullptr, false);

    ServerUtils::setThreadOptions(ctx.get(), TESTMODE);

    fs::path tmpDir = Paths::getTmpDir(projectName);
    clearDirectory(tmpDir);
    fs::create_directories(tmpDir);
}

void BaseTest::clearTestDirectory() {
    clearDirectory(getTestDirectory());
}

void BaseTest::clearDirectory(const fs::path &pathToDirectory) {
    FileSystemUtils::removeAll(pathToDirectory);
}

fs::path BaseTest::getTestDirectory() {
    return suitePath / "tests";
}

fs::path BaseTest::getPathToGeneratedTestFileByTestedFile(const string &fileName) {
    return getTestDirectory() /
           (Paths::addExtension(Paths::addTestSuffix(Paths::removeExtension(fileName)), ".cpp"));
}

fs::path BaseTest::getPathToGeneratedTestHeaderFileByTestedFile(const string &fileName) {
    return getTestDirectory() /
           (Paths::addExtension(Paths::addTestSuffix(Paths::removeExtension(fileName)), ".h"));
}

fs::path BaseTest::getStubsDirectory() {
    return getTestDirectory() / "stubs";
}
