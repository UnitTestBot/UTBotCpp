/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "AssertionTestGen.h"

#include "utils/ExecUtils.h"

AssertionTestGen::AssertionTestGen(const testsgen::AssertionRequest &request,
                                   ProgressWriter *progressWriter,
                                   bool testMode)
    : LineTestGen(request.linerequest(), progressWriter, testMode) {
}

string AssertionTestGen::toString() {
    return LineTestGen::toString();
}
