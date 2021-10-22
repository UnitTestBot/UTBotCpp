/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "CoverageAndResultsGenerator.h"

#include "TimeExecStatistics.h"
#include "exceptions/CoverageGenerationException.h"
#include "utils/StringUtils.h"

#include "loguru.hpp"

using grpc::Status;
using grpc::StatusCode;
using std::string;
using std::vector;

CoverageAndResultsGenerator::CoverageAndResultsGenerator(
    testsgen::CoverageAndResultsRequest const *coverageAndResultsRequest,
    CoverageAndResultsWriter *coverageAndResultsWriter)
    : TestRunner(coverageAndResultsRequest->projectcontext(),
                 coverageAndResultsRequest->testfilter().testfilepath(),
                 coverageAndResultsRequest->testfilter().testsuite(),
                 coverageAndResultsRequest->testfilter().testname(),
                 coverageAndResultsWriter),
      coverageAndResultsWriter(coverageAndResultsWriter) {
}

grpc::Status CoverageAndResultsGenerator::generate(bool withCoverage,
                                                   testsgen::SettingsContext &settingsContext) {
    auto context = utbot::SettingsContext(settingsContext);
    return generate(withCoverage, context);
}

grpc::Status CoverageAndResultsGenerator::generate(bool withCoverage,
                                                   utbot::SettingsContext &settingsContext) {
    MEASURE_FUNCTION_EXECUTION_TIME
    try {
        init(withCoverage);
        runTests(withCoverage, settingsContext.timeoutPerTest);
        if (withCoverage) {
            collectCoverage();
        }
    } catch (CoverageGenerationException &e) {
        showErrors();
        return Status(StatusCode::FAILED_PRECONDITION, e.what());
    } catch (ExecutionProcessException &e) {
        exceptions.emplace_back(e);
        showErrors();
        return Status(StatusCode::FAILED_PRECONDITION, e.what());
    } catch (CancellationException &e) {
        return Status::CANCELLED;
    }

    showErrors();
    return Status::OK;
}

void CoverageAndResultsGenerator::showErrors() const {
    std::optional<fs::path> errorMessage;
    if (hasExceptions()) {
        auto log = StringUtils::joinWith(exceptions, "\n");
        fs::path logFilePath =
            LogUtils::writeLog(log, projectContext.projectName, "Collection_coverage");
        auto message =
            StringUtils::stringFormat("%d actions failed during building and running tests. "
                                      "See more info here: "
                                      "%s",
                                      exceptions.size(), logFilePath);
        LOG_S(WARNING) << message;
        errorMessage = message;
    }

    coverageAndResultsWriter->writeResponse(testStatusMap, coverageMap, totals, errorMessage);
}

Coverage::CoverageMap const &CoverageAndResultsGenerator::getCoverageMap() {
    return coverageMap;
}

nlohmann::json const &CoverageAndResultsGenerator::getTotals() {
    return totals;
}

void CoverageAndResultsGenerator::collectCoverage() {
    MEASURE_FUNCTION_EXECUTION_TIME
    if (testsToLaunch.empty()) {
        return;
    }
    std::vector<ShellExecTask> coverageCommands = coverageTool->getCoverageCommands(
        CollectionUtils::filterToVector(testsToLaunch, [this](const UnitTest &testToLaunch) {
            return testStatusMap[testToLaunch.testFilePath][testToLaunch.testname] !=
                   testsgen::TEST_INTERRUPTED;
        }));
    if (coverageCommands.empty()) {
        return;
    }
    ExecUtils::doWorkWithProgress(
        coverageCommands, coverageAndResultsWriter, "Collecting coverage",
        [this](ShellExecTask &task) {
            auto [out, status, path] = task.run();
            if (status != 0) {
                exceptions.emplace_back(
                    StringUtils::stringFormat("Command: %s\nOutput: %s", task.toString(), out),
                    path.value());
            }
        });

    LOG_S(DEBUG) << "All coverage commands were executed";

    coverageMap = coverageTool->getCoverageInfo();
    totals = coverageTool->getTotals();
}
