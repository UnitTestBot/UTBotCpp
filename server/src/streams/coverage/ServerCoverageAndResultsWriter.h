/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */


#ifndef UNITTESTBOT_SERVERCOVERAGEANDRESULTSWRITER_H
#define UNITTESTBOT_SERVERCOVERAGEANDRESULTSWRITER_H

#include "SettingsContext.h"
#include "coverage/Coverage.h"
#include "streams/WriterUtils.h"
#include "streams/coverage/CoverageAndResultsWriter.h"

#include <protobuf/testgen.pb.h>

#include <utility>

class ServerCoverageAndResultsWriter : public CoverageAndResultsWriter {
public:
    explicit ServerCoverageAndResultsWriter(
        grpc::ServerWriter<testsgen::CoverageAndResultsResponse> *writer);

    virtual void writeResponse(const Coverage::TestStatusMap &testsStatusMap,
                               const Coverage::CoverageMap &coverageMap,
                               const nlohmann::json &totals,
                               std::optional<string> errorMessage) override;
};


#endif // UNITTESTBOT_SERVERCOVERAGEANDRESULTSWRITER_H
