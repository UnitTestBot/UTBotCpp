/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "FunctionTestGen.h"

#include "utils/ExecUtils.h"

FunctionTestGen::FunctionTestGen(const testsgen::FunctionRequest &request,
                                 ProgressWriter *progressWriter,
                                 bool testMode)
    : LineTestGen(request.linerequest(), progressWriter, testMode) {
}

string FunctionTestGen::toString() {
    return LineTestGen::toString();
}

bool FunctionTestGen::needToAddPathFlag() {
    return false;
}
