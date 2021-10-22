/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_VSCODETESTSWRITER_H
#define UNITTESTBOT_VSCODETESTSWRITER_H

#include "Tests.h"
#include "TestsWriter.h"
#include "streams/IStreamWriter.h"

#include "loguru.hpp"

#include <utils/FileSystemUtils.h>

class ServerTestsWriter : public TestsWriter {
public:
    explicit ServerTestsWriter(grpc::ServerWriter<testsgen::TestsResponse> *writer,
                               bool synchronizeCode)
        : TestsWriter(writer), synchronizeCode(synchronizeCode)  {};

    void writeTestsWithProgress(tests::TestsMap &testMap,
                                std::string const &message,
                                const fs::path &testDirPath,
                                std::function<void(tests::Tests &)> &&functor) override;

    void writeStubs(const vector<Stubs> &synchronizedStubs) override;

private:
    [[nodiscard]] virtual bool writeFileAndSendResponse(const tests::Tests &tests,
                                                        const fs::path &testDirPath,
                                                        const std::string &message,
                                                        double percent,
                                                        bool isCompleted) const;

    bool synchronizeCode;
};


#endif // UNITTESTBOT_VSCODETESTSWRITER_H
