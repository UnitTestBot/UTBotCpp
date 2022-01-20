/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "FolderTestGen.h"

#include "Paths.h"

FolderTestGen::FolderTestGen(const testsgen::FolderRequest &request,
                             ProgressWriter *progressWriter,
                             bool testMode)
    : ProjectTestGen(request.projectrequest(), progressWriter, testMode, false),
      folderPath(request.folderpath()) {
    testingMethodsSourcePaths = {};
    std::copy_if(sourcePaths.begin(), sourcePaths.end(),
                 std::back_inserter(testingMethodsSourcePaths),
                 [this](const fs::path &path) { return Paths::isSubPathOf(folderPath, path); });
    setInitializedTestsMap();
}

string FolderTestGen::toString() {
    std::stringstream s;
    s << ProjectTestGen::toString() << "folder path: " << folderPath << "\nfile paths:\n";
    for (const auto &sp : testingMethodsSourcePaths) {
        s << sp.string() << "; ";
    }
    s << "\n";
    return s.str();
}
