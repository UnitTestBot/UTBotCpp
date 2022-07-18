#include "BaseTestGen.h"

#include "FileTestGen.h"
#include "FolderTestGen.h"
#include "LineTestGen.h"
#include "utils/ExecUtils.h"
#include "utils/ServerUtils.h"
#include "utils/TypeUtils.h"

BaseTestGen::BaseTestGen(const testsgen::ProjectContext &projectContext,
                         const testsgen::SettingsContext &settingsContext,
                         ProgressWriter *progressWriter,
                         bool testMode)
    : projectContext(projectContext),
      settingsContext(settingsContext), progressWriter(progressWriter) {
    serverBuildDir = Paths::getUtbotBuildDir(this->projectContext);
}

bool BaseTestGen::needToBeMocked() const {
    return TypeUtils::isSameType<LineTestGen>(*this) ||
           TypeUtils::isSameType<FileTestGen>(*this) ||
           TypeUtils::isSameType<FolderTestGen>(*this);
}

bool BaseTestGen::isBatched() const {
    return TypeUtils::isSameType<FolderTestGen>(*this) ||
           TypeUtils::isSameType<ProjectTestGen>(*this);
}

void BaseTestGen::setInitializedTestsMap() {
    tests.clear();
    for (const fs::path & sourcePath : testingMethodsSourcePaths) {
        tests::Tests testsSuite;
        testsSuite.sourceFilePath = sourcePath;
        testsSuite.sourceFileNameNoExt = testsSuite.sourceFilePath.stem().string();
        testsSuite.relativeFileDir =
            Paths::getRelativeDirPath(projectContext, testsSuite.sourceFilePath);
        testsSuite.testFilename = Paths::sourcePathToTestName(testsSuite.sourceFilePath);
        testsSuite.testHeaderFilePath =
            Paths::getGeneratedHeaderPath(projectContext, testsSuite.sourceFilePath);
        testsSuite.testSourceFilePath =
            Paths::sourcePathToTestPath(projectContext, testsSuite.sourceFilePath);
        tests[testsSuite.sourceFilePath] = testsSuite;
    }
}

void BaseTestGen::setTargetPath(fs::path _targetPath) {
    if (targetPath != _targetPath) {
        targetPath = std::move(_targetPath);
        if (!hasAutoTarget()) {
            updateTargetSources();
        }
    }
}

void BaseTestGen::updateTargetSources() {
    targetSources = CollectionUtils::transformTo<CollectionUtils::FileSet>(
            buildDatabase->getArchiveObjectFiles(getTargetPath()), [this](fs::path const &objectPath) {
            return buildDatabase->getClientCompilationUnitInfo(objectPath)->getSourcePath();
        });
    for (auto it = tests.begin(); it != tests.end(); it++) {
        tests::Tests &test = it.value();
        test.isFilePresentedInCommands = CollectionUtils::contains(targetSources, test.sourceFilePath);
    }
}

fs::path const &BaseTestGen::getTargetPath() const {
    return targetPath.value();
}

bool BaseTestGen::hasAutoTarget() const {
    return getTargetPath() == GrpcUtils::UTBOT_AUTO_TARGET_PATH;
}
