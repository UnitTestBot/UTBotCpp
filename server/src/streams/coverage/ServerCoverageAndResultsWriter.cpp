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

        for (const auto &sourceLine : coverage.fullCoverageLines) {
            auto sourceLineGrpc = fileCoverageGrpc->add_fullcoveragelines();
            writeSourceLine(sourceLineGrpc, sourceLine);
        }
        for (const auto &sourceLine : coverage.partialCoverageLines) {
            auto sourceLineGrpc = fileCoverageGrpc->add_partialcoveragelines();
            writeSourceLine(sourceLineGrpc, sourceLine);
        }
        for (const auto &sourceLine : coverage.noCoverageLines) {
            auto sourceLineGrpc = fileCoverageGrpc->add_nocoveragelines();
            writeSourceLine(sourceLineGrpc, sourceLine);
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
