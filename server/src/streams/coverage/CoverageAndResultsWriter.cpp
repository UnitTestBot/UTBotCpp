/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "CoverageAndResultsWriter.h"

CoverageAndResultsWriter::CoverageAndResultsWriter(
    grpc::ServerWriter<testsgen::CoverageAndResultsResponse> *writer)
    : ServerWriter(writer) {
}
