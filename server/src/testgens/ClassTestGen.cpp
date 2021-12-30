/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "ClassTestGen.h"

ClassTestGen::ClassTestGen(const testsgen::ClassRequest &request,
                           ProgressWriter *progressWriter,
                           bool testMode)
    : LineTestGen(request.linerequest(), progressWriter, testMode, true) {
}

string ClassTestGen::toString() {
    return LineTestGen::toString();
}

bool ClassTestGen::needToAddPathFlag() {
    return false;
}
