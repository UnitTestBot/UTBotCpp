/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "TestsWriter.h"

#include "utils/FileSystemUtils.h"

#include "loguru.hpp"


TestsWriter::TestsWriter(grpc::ServerWriter<testsgen::TestsResponse> *writer): ServerWriter(writer) {}

void TestsWriter::writeCompleted(const tests::TestsMap &testMap, int totalTestsCounter) {
    std::string finalMessage;
    if (totalTestsCounter == 1) {
        finalMessage = StringUtils::stringFormat("%d test file generated.", totalTestsCounter);
    } else {
        finalMessage = StringUtils::stringFormat("%d test files generated.", totalTestsCounter);
    }
    writeProgress(finalMessage, 100.0, true);
}
