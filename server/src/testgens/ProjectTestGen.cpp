#include "ProjectTestGen.h"

#include "Paths.h"
#include "building/BuildDatabase.h"
#include "exceptions/CompilationDatabaseException.h"
#include "utils/CompilationUtils.h"

ProjectTestGen::ProjectTestGen(const testsgen::ProjectRequest &request,
                               ProgressWriter *progressWriter,
                               bool testMode,
                               bool autoDetect,
                               const std::optional<fs::path> &sourceFile)
        : BaseTestGen(request.projectcontext(),
                      request.settingscontext(),
                      progressWriter,
                      testMode), request(&request) {
    fs::create_directories(projectContext.testDirPath);
    compileCommandsJsonPath = CompilationUtils::substituteRemotePathToCompileCommandsJsonPath(projectContext);
    projectBuildDatabase = std::make_shared<ProjectBuildDatabase>(compileCommandsJsonPath, serverBuildDir,
                                                                  projectContext,
                                                                  settingsContext.skipObjectWithoutSource);
    if (sourceFile.has_value() && Paths::isSourceFile(sourceFile.value()) &&
        (request.targetpath() == GrpcUtils::UTBOT_AUTO_TARGET_PATH || request.targetpath().empty())) {
        targetBuildDatabase = std::make_shared<TargetBuildDatabase>(projectBuildDatabase.get(), sourceFile.value());
    } else {
        targetBuildDatabase = std::make_shared<TargetBuildDatabase>(projectBuildDatabase.get(), request.targetpath());
    }
    if (autoDetect) {
        autoDetectSourcePathsIfNotEmpty();
    } else {
        sourcePaths = targetBuildDatabase->compilationDatabase->getAllFiles();
    }
    testingMethodsSourcePaths = sourcePaths;
    setInitializedTestsMap();
    updateTargetSources(targetBuildDatabase->getTargetPath());
}

std::string ProjectTestGen::toString() {
    std::stringstream s;
    s << request->projectcontext().DebugString() << "\n";
    s << request->settingscontext().DebugString() << "\n";
    return s.str();
}

void ProjectTestGen::setTargetForSource(const fs::path &sourcePath) {
    fs::path root = targetBuildDatabase->getRootForSource(sourcePath);
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
    auto sourcePathsCandidates = targetBuildDatabase->compilationDatabase->getAllFiles();
    if (!requestSourcePaths.empty()) {
        sourcePaths =
                Paths::filterPathsByDirNames(sourcePathsCandidates, requestSourcePaths, Paths::isSourceFile);
    } else {
        sourcePaths = sourcePathsCandidates;
    }
}
