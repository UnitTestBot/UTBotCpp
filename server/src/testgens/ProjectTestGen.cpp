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
                               bool testMode)
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
    auto requestSourcePaths = CollectionUtils::transformTo<vector<fs::path>>(
        request.sourcepaths(), [](std::string const &sourcePath) { return fs::path(sourcePath); });
    sourcePaths =
        Paths::filterPathsByDirNames(sourcePathsCandidates, requestSourcePaths, { ".c", ".cpp", ".cc" });
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
