/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "WriterUtils.h"
#include "Paths.h"

void writeSourceRange(testsgen::SourceRange *sourceRangeGrpc, Coverage::FileCoverage::SourceRange sourceRange) {
    auto startPosition = std::make_unique<testsgen::SourcePosition>();
    startPosition->set_line(sourceRange.start.line);
    startPosition->set_character(sourceRange.start.character);
    auto endPosition = std::make_unique<testsgen::SourcePosition>();
    endPosition->set_line(sourceRange.end.line);
    endPosition->set_character(sourceRange.end.character);

    sourceRangeGrpc->set_allocated_start(startPosition.release());
    sourceRangeGrpc->set_allocated_end(endPosition.release());
}
