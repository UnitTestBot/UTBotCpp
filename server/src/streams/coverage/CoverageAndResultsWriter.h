/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_COVERAGEJSONWRITER_H
#define UNITTESTBOT_COVERAGEJSONWRITER_H

#include "streams/ServerWriter.h"
#include "coverage/Coverage.h"
#include "exceptions/ExecutionProcessException.h"

#include <grpcpp/grpcpp.h>
#include "json.hpp"
#include <protobuf/testgen.grpc.pb.h>


class CoverageAndResultsWriter : public utbot::ServerWriter<testsgen::CoverageAndResultsResponse> {
public:

    explicit CoverageAndResultsWriter(grpc::ServerWriter<testsgen::CoverageAndResultsResponse> *writer);

    virtual void writeResponse(const Coverage::TestStatusMap &testsStatusMap,
                               const Coverage::CoverageMap &coverageMap,
                               const nlohmann::json &totals,
                               std::optional<string> errorMessage) = 0;
};


#endif // UNITTESTBOT_COVERAGEJSONWRITER_H
