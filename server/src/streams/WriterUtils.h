/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_WRITERUTILS_H
#define UNITTESTBOT_WRITERUTILS_H

#include "coverage/Coverage.h"
#include "stubs/Stubs.h"

#include <protobuf/testgen.grpc.pb.h>

void writeSourceRange(testsgen::SourceRange *sourceRangeGrpc, Coverage::FileCoverage::SourceRange sourceRange);

#endif //UNITTESTBOT_WRITERUTILS_H
