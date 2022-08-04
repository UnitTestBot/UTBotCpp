#include "ProjectTestGen.h"

#include "Paths.h"
#include "building/BuildDatabase.h"
#include "exceptions/CompilationDatabaseException.h"
#include "utils/CompilationUtils.h"

ProjectTestGen::ProjectTestGen(const testsgen::ProjectRequest &request,
                               ProgressWriter *progressWriter,
                               bool testMode,
                               bool autoSrcPaths)
        : BaseTestGen(request.projectcontext(),
                      request.settingscontext(),
                      progressWriter,
                      testMode), request(&request) {
    fs::create_directories(projectContext.testDirPath);
    compileCommandsJsonPath = CompilationUtils::substituteRemotePathToCompileCommandsJsonPath(
            projectContext.projectPath, projectContext.buildDirRelativePath);
    baseBuildDatabase = std::make_shared<BuildDatabase>(compileCommandsJsonPath, serverBuildDir, projectContext, false);
    buildDatabase = baseBuildDatabase->createBaseForTarget(request.targetpath());
//    compilationDatabase = CompilationUtils::getCompilationDatabase(compileCommandsJsonPath);
    if (autoSrcPaths) {
        autoDetectSourcePathsIfNotEmpty();
    } else {
        sourcePaths = buildDatabase->compilationDatabase->getAllFiles();
    }
    testingMethodsSourcePaths = sourcePaths;
    setInitializedTestsMap();
    updateTargetSources(buildDatabase->getTargetPath());
}

std::string ProjectTestGen::toString() {
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

std::vector<fs::path> ProjectTestGen::getRequestSourcePaths() const {
    return CollectionUtils::transformTo<std::vector<fs::path>>(
            request->sourcepaths(), [](std::string const &sourcePath) { return fs::path(sourcePath); });
}

void ProjectTestGen::autoDetectSourcePathsIfNotEmpty() {
    // requestSourcePaths are from settings.json
    auto requestSourcePaths = getRequestSourcePaths();
    // sourcePathsCandidates are from compile_commands.json
    auto sourcePathsCandidates = buildDatabase->compilationDatabase->getAllFiles();
    if (!requestSourcePaths.empty()) {
        sourcePaths =
                Paths::filterPathsByDirNames(sourcePathsCandidates, requestSourcePaths, Paths::isSourceFile);
    } else {
        sourcePaths = sourcePathsCandidates;
    }
}
