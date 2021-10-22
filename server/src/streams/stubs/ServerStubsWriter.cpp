/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "ServerStubsWriter.h"

#include "loguru.h"

void ServerStubsWriter::writeResponse(const vector<Stubs> &synchronizedStubs,
                                      const fs::path &testDirPath) {
    writeStubsFilesOnServer(synchronizedStubs, testDirPath);
    if (!hasStream()) {
        return;
    }
    testsgen::StubsResponse response;
    LOG_S(DEBUG) << "Creating final response.";
    for (const auto &synchronizedStub : synchronizedStubs) {
        auto sData = response.add_stubsources();
        sData->set_filepath(synchronizedStub.filePath);
        if (synchronizeCode) {
            sData->set_code(synchronizedStub.code);
        }
    }
    auto progress = GrpcUtils::createProgress(std::nullopt, 0, true);
    response.set_allocated_progress(progress.release());
    writeMessage(response);
}


