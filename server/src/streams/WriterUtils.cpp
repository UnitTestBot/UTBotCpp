/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "WriterUtils.h"

void writeSourceLine(testsgen::SourceLine *sourceLineGrpc, Coverage::FileCoverage::SourceLine sourceLine) {
    sourceLineGrpc->set_line(sourceLine.line);
}
