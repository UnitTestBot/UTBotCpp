#include "SnippetTestGen.h"

#include "printers/CCJsonPrinter.h"
#include "utils/CompilationUtils.h"

const std::string SNIPPET_TARGET = "executable";

SnippetTestGen::SnippetTestGen(const testsgen::SnippetRequest &request,
                               ProgressWriter *progressWriter,
                               bool testMode)
        : BaseTestGen(request.projectcontext(),
                      request.settingscontext(),
                      progressWriter,
                      testMode) {
    filePath = fs::weakly_canonical(request.filepath());
    sourcePaths = {filePath};
    testingMethodsSourcePaths = sourcePaths;
    printer::CCJsonPrinter::createDummyBuildDB(sourcePaths, serverBuildDir);
    compileCommandsJsonPath = serverBuildDir;
    utbot::ProjectContext projectContext{request, serverBuildDir};
    baseBuildDatabase = std::make_shared<BuildDatabase>(compileCommandsJsonPath, serverBuildDir, projectContext);
    buildDatabase = baseBuildDatabase->createBuildDatabaseForSourceOrTarget(serverBuildDir / SNIPPET_TARGET);
    setTargetForSource(filePath);
    setInitializedTestsMap();
}

std::string SnippetTestGen::toString() {
    return StringUtils::stringFormat("\tfilepath: %s\n", filePath);
}

void SnippetTestGen::setTargetForSource(const fs::path &sourcePath) {
    fs::path root = serverBuildDir / SNIPPET_TARGET;
    setTargetPath(root);
}
