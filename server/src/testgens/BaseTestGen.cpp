#include "BaseTestGen.h"

#include "exceptions/CompilationDatabaseException.h"
#include "FileTestGen.h"
#include "FolderTestGen.h"
#include "LineTestGen.h"
#include "utils/ExecUtils.h"
#include "utils/ServerUtils.h"
#include "utils/TypeUtils.h"
#include "loguru.h"

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
    for (const fs::path &sourcePath: testingMethodsSourcePaths) {
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
    if (targetBuildDatabase->hasAutoTarget() && targetBuildDatabase->getTargetPath() != _targetPath) {
        targetBuildDatabase = std::make_shared<TargetBuildDatabase>(projectBuildDatabase.get(), _targetPath);
        updateTargetSources(_targetPath);
    }
}

void BaseTestGen::updateTargetSources(fs::path _targetPath) {
    targetSources = targetBuildDatabase->getSourceFilesForTarget(_targetPath);
    for (auto it = tests.begin(); it != tests.end(); it++) {
        tests::Tests &test = it.value();
        test.isFilePresentedInCommands = CollectionUtils::contains(targetSources, test.sourceFilePath);
    }
}

std::shared_ptr<const ProjectBuildDatabase> BaseTestGen::getProjectBuildDatabase() const {
    return projectBuildDatabase;
}

std::shared_ptr<const TargetBuildDatabase> BaseTestGen::getTargetBuildDatabase() const {
    return targetBuildDatabase;
}

std::shared_ptr<ProjectBuildDatabase> BaseTestGen::getProjectBuildDatabase() {
    return projectBuildDatabase;
}

std::shared_ptr<TargetBuildDatabase> BaseTestGen::getTargetBuildDatabase() {
    return targetBuildDatabase;
}

std::shared_ptr<const BuildDatabase::ObjectFileInfo>
BaseTestGen::getClientCompilationUnitInfo(const fs::path &path, bool fullProject) const {
    std::shared_ptr<const BuildDatabase::ObjectFileInfo> objectFileInfo;
    if (targetBuildDatabase->hasUnitInfo(path) || !fullProject) {
        objectFileInfo = targetBuildDatabase->getClientCompilationUnitInfo(path);
    } else {
        objectFileInfo = projectBuildDatabase->getClientCompilationUnitInfo(path);
        LOG_S(WARNING) << "Can't find in target: " << path;
    }
    return objectFileInfo;
}
