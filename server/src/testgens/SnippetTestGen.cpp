/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "SnippetTestGen.h"

#include "printers/CCJsonPrinter.h"
#include "utils/CompilationUtils.h"

SnippetTestGen::SnippetTestGen(const testsgen::SnippetRequest &request,
                               ProgressWriter *progressWriter,
                               bool testMode)
    : BaseTestGen(request.projectcontext(),
                  request.settingscontext(),
                  progressWriter,
                  testMode) {
    filePath = fs::weakly_canonical(request.filepath());
    sourcePaths = { filePath };
    testingMethodsSourcePaths = sourcePaths;
    printer::CCJsonPrinter::createDummyBuildDB(sourcePaths, serverBuildDir);
    compileCommandsJsonPath = serverBuildDir;
    utbot::ProjectContext projectContext{ request, serverBuildDir };
    buildDatabase =
        std::make_shared<BuildDatabase>(compileCommandsJsonPath, serverBuildDir, projectContext);
    compilationDatabase = CompilationUtils::getCompilationDatabase(serverBuildDir);
    setTargetForSource(filePath);
    setInitializedTestsMap();
}

std::string SnippetTestGen::toString() {
    return StringUtils::stringFormat("\tfilepath: %s\n", filePath);
}

void SnippetTestGen::setTargetForSource(const fs::path &sourcePath) {
    fs::path root = serverBuildDir / "executable";
    setTargetPath(root);
}

