/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "FileTestGen.h"

#include "utils/ExecUtils.h"

FileTestGen::FileTestGen(const testsgen::FileRequest &request,
                         ProgressWriter *progressWriter,
                         bool testMode)
    : ProjectTestGen(request.projectrequest(), progressWriter, testMode),
      filepath(request.filepath()) {
    testingMethodsSourcePaths = { filepath  };
    setInitializedTestsMap();
}

string FileTestGen::toString() {
    std::stringstream s;
    s << ProjectTestGen::toString() << "\n\tfilepath: " << filepath << "\n";
    return s.str();
}

void FileTestGen::setModuleTestingSources(const vector<fs::path> &moduleFiles) {
    testingMethodsSourcePaths = moduleFiles;
    setInitializedTestsMap();
}
