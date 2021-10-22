/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */


#include "ServerCoverageAndResultsWriter.h"

#include "../WriterUtils.h"

#include "loguru.h"

ServerCoverageAndResultsWriter::ServerCoverageAndResultsWriter(
    grpc::ServerWriter<testsgen::CoverageAndResultsResponse> *writer)
    : CoverageAndResultsWriter(writer) {
}

void ServerCoverageAndResultsWriter::writeResponse(const Coverage::TestStatusMap &testsStatusMap,
                                                   const Coverage::CoverageMap &coverageMap,
                                                  const nlohmann::json &totals,
                                                   std::optional<string> errorMessage) {
    if (!hasStream()) {
        return;
    }
    LOG_S(INFO) << "Creating coverage response.";

    testsgen::CoverageAndResultsResponse response;

    for (const auto &[filepath, fileTestsStatus] : testsStatusMap) {
        for (const auto &[testname, status] : fileTestsStatus) {
            auto testResultsGrpc = response.add_testrunresults();
            testResultsGrpc->set_testfilepath(filepath);
            testResultsGrpc->set_testname(testname);
            testResultsGrpc->set_status(status);
            testResultsGrpc->set_output("");
        }
    }


    for (const auto &[filepath, coverage] : coverageMap) {
        auto fileCoverageGrpc = response.add_coverages();
        fileCoverageGrpc->set_filepath(filepath);

        for (const auto &sourceRange : coverage.coveredRanges) {
            auto sourceRangeGrpc = fileCoverageGrpc->add_coveredranges();
            writeSourceRange(sourceRangeGrpc, sourceRange);
        }
        for (const auto &sourceRange : coverage.uncoveredRanges) {
            auto sourceRangeGrpc = fileCoverageGrpc->add_uncoveredranges();
            writeSourceRange(sourceRangeGrpc, sourceRange);
        }
    }

    auto message = "Coverage response generated";
    LOG_S(INFO) << message;
    auto progress = GrpcUtils::createProgress(message, 0, true);
    response.set_allocated_progress(progress.release());
    if (errorMessage.has_value()) {
        response.set_errormessage(errorMessage.value());
    }
    writeMessage(response);
}
