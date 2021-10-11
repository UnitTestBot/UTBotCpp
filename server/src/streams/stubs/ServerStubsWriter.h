/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_VSCODESTUBSWRITER_H
#define UNITTESTBOT_VSCODESTUBSWRITER_H

#include "StubsWriter.h"

class ServerStubsWriter : public StubsWriter {
public:
    explicit ServerStubsWriter(grpc::ServerWriter<testsgen::StubsResponse> *writer, bool synchronizeCode)
        : StubsWriter(writer), synchronizeCode(synchronizeCode) {
    }

    void writeResponse(const vector<Stubs> &synchronizedStubs,
                       const fs::path &testDirPath) override;
private:
    bool synchronizeCode;
};


#endif // UNITTESTBOT_VSCODESTUBSWRITER_H
