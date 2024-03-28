#include "BaseTest.h"

#include "utils/FileSystemUtils.h"
#include "utils/ServerUtils.h"

BaseTest::BaseTest(const std::string &suiteName) {
    setSuite(suiteName);
}

void BaseTest::setSuite(const std::string &suite) {
    suiteName = suite;
    suitePath = baseSuitePath / suiteName;
    srcPaths = { suitePath, suitePath / "inner" };
    setCompiler(compilerName);
}

void BaseTest::setCompiler(CompilationUtils::CompilerName name) {
    compilerName = name;
    setBuildDirectory(getBuildDirectoryName(compilerName));
}

void BaseTest::setBuildDirectory(const std::string &buildDirectoryName) {
    buildDirRelPath = buildDirectoryName;
    buildPath = suitePath / buildDirRelPath;
}

fs::path BaseTest::getTestFilePath(const std::string &fileName) {
    return suitePath / fileName;
}

void BaseTest::clearEnv(const CompilationUtils::CompilerName& compiler) {
    compilerName = compiler;
    setSuite(suiteName);

    ctx = std::make_unique<ServerContext>();
    writer = std::make_unique<ServerTestsWriter>(nullptr, false);

    ServerUtils::setThreadOptions(ctx.get(), TESTMODE);

    fs::path utbotBuildDir = buildPath / CompilationUtils::UTBOT_FILES_DIR_NAME;
    clearDirectory(utbotBuildDir);
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

fs::path BaseTest::getPathToGeneratedTestFileByTestedFile(const std::string &fileName) {
    return getTestDirectory() /
           (Paths::addExtension(
               Paths::addTestSuffix(
                   Paths::addSuffix(Paths::removeExtension(fileName), "_dot_c")),
               ".cpp"));
}

fs::path BaseTest::getPathToGeneratedTestHeaderFileByTestedFile(const std::string &fileName) {
    return getTestDirectory() /
           (Paths::addExtension(
               Paths::addTestSuffix(
                   Paths::addSuffix(Paths::removeExtension(fileName), "_dot_c")),
               ".h"));
}

fs::path BaseTest::getStubsDirectory() {
    return getTestDirectory() / "stubs";
}
