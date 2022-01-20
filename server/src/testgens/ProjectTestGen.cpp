/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "ProjectTestGen.h"

#include "Paths.h"
#include "building/BuildDatabase.h"
#include "exceptions/CompilationDatabaseException.h"
#include "utils/CompilationUtils.h"

ProjectTestGen::ProjectTestGen(const testsgen::ProjectRequest &request,
                               ProgressWriter *progressWriter,
                               bool testMode,
                               bool autoDetect)
    : BaseTestGen(request.projectcontext(),
                  request.settingscontext(),
                  progressWriter,
                  testMode), request(&request) {
    fs::create_directories(projectContext.testDirPath);
    compileCommandsJsonPath = CompilationUtils::substituteRemotePathToCompileCommandsJsonPath(
        projectContext.projectPath, projectContext.buildDirRelativePath);
    buildDatabase =
        std::make_shared<BuildDatabase>(compileCommandsJsonPath, serverBuildDir, projectContext);
    compilationDatabase = CompilationUtils::getCompilationDatabase(compileCommandsJsonPath);
    if (autoDetect) {
        autoDetectSourcePathsIfNotEmpty();
    } else {
        vector<fs::path> sourcePathsCandidates = getSourcePathCandidates();
        sourcePaths = sourcePathsCandidates;
    }
    testingMethodsSourcePaths = sourcePaths;
    setInitializedTestsMap();
}

string ProjectTestGen::toString() {
    std::stringstream s;
    s << request->projectcontext().DebugString() << "\n";
    s << request->settingscontext().DebugString() << "\n";
    return s.str();
}

void ProjectTestGen::setTargetForSource(const fs::path &sourcePath) {
    fs::path root = buildDatabase->getRootForSource(sourcePath);
    setTargetPath(root);
}

const testsgen::ProjectRequest *ProjectTestGen::getRequest() const {
    return request;
}

vector<fs::path> ProjectTestGen::getRequestSourcePaths() const {
    return CollectionUtils::transformTo<vector<fs::path>>(
            request->sourcepaths(), [](std::string const &sourcePath) { return fs::path(sourcePath); });
}

vector<fs::path> ProjectTestGen::getSourcePathCandidates() const {
    vector<fs::path> sourcePathsCandidates;
    for (const auto &compileCommand : compilationDatabase->getAllCompileCommands()) {
        try {
            fs::path path = Paths::getCCJsonFileFullPath(compileCommand.Filename, compileCommand.Directory);
            sourcePathsCandidates.push_back(path);
        } catch (...) {
            throw CompilationDatabaseException("Cannot detect file: " + compileCommand.Filename +
            ". Maybe you need to rebuild the project.");
        }
    }
    return sourcePathsCandidates;
}
void ProjectTestGen::autoDetectSourcePathsIfNotEmpty() {
    // requestSourcePaths are from settings.json
    auto requestSourcePaths = getRequestSourcePaths();
    // sourcePathsCandidates are from compile_commands.json
    auto sourcePathsCandidates = getSourcePathCandidates();
    if (!requestSourcePaths.empty()) {
        sourcePaths =
                Paths::filterPathsByDirNames(sourcePathsCandidates, requestSourcePaths, Paths::isSourceFile);
    } else {
        sourcePaths = sourcePathsCandidates;
    }
}


