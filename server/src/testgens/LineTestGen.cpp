/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "LineTestGen.h"

LineTestGen::LineTestGen(const testsgen::LineRequest &request,
                         ProgressWriter *progressWriter,
                         bool testMode, bool forHeader)
        : ProjectTestGen(request.projectrequest(), progressWriter, testMode, false) {
    filePath = request.sourceinfo().filepath();
    line = request.sourceinfo().line();
    std::optional<fs::path> sourcePath = Paths::headerPathToSourcePath(filePath);
    if (forHeader && sourcePath.has_value()) {
        testingMethodsSourcePaths = {sourcePath.value()};
    } else {
        testingMethodsSourcePaths = {filePath};
    }
    setInitializedTestsMap();
}

string LineTestGen::toString() {
    std::stringstream s;
    s << ProjectTestGen::toString() << "\tline info:"
      << "\n\t\tfilepath: " << filePath << "\n\t\tline: " << line << "\n";
    return s.str();
}

bool LineTestGen::needToAddPathFlag() {
    return true;
}
