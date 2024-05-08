#include "CoverageAndResultsGenerator.h"

#include "TimeExecStatistics.h"
#include "exceptions/CoverageGenerationException.h"
#include "utils/FileSystemUtils.h"
#include "utils/StringUtils.h"
#include "utils/stats/TestsExecutionStats.h"

#include "loguru.h"

using grpc::Status;
using grpc::StatusCode;

CoverageAndResultsGenerator::CoverageAndResultsGenerator(
    testsgen::CoverageAndResultsRequest const *coverageAndResultsRequest,
    CoverageAndResultsWriter *coverageAndResultsWriter)
    : TestRunner(utbot::ProjectContext(coverageAndResultsRequest->projectcontext()),
                 coverageAndResultsRequest->testfilter().testfilepath(),
                 coverageAndResultsRequest->testfilter().testsuite(),
                 coverageAndResultsRequest->testfilter().testname(),
                 coverageAndResultsRequest->testfilter().functionname(),
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
            StatsUtils::TestsExecutionStatsFileMap testsExecutionStats(projectContext, testResultMap, coverageMap);
            printer::CSVPrinter printer = testsExecutionStats.toCSV();
            FileSystemUtils::writeToFile(Paths::getExecutionStatsCSVPath(projectContext), printer.getStream().str());
            LOG_S(INFO) << StringUtils::stringFormat("See execution stats here: %s",
                                                     Paths::getExecutionStatsCSVPath(projectContext));
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

    coverageAndResultsWriter->writeResponse(projectContext, testResultMap, coverageMap, totals, errorMessage);
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
            return testResultMap[testToLaunch.testFilePath][testToLaunch.testname].status() !=
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
