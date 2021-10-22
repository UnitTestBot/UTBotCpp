/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "CLIStubsWriter.h"

#include <utils/FileSystemUtils.h>
#include "loguru.hpp"

void CLIStubsWriter::writeResponse(const vector<Stubs> &synchronizedStubs,
                                   const fs::path &testDirPath) {
    LOG_S(INFO) << "Writing stubs...";
    writeStubsFilesOnServer(synchronizedStubs, testDirPath);
    LOG_S(INFO) << "Stubs generated";
}
