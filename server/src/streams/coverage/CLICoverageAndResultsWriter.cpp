#include "CLICoverageAndResultsWriter.h"

#include "utils/FileSystemUtils.h"

#include "loguru.h"
#include "Paths.h"

CLICoverageAndResultsWriter::CLICoverageAndResultsWriter()
    : CoverageAndResultsWriter(nullptr) {
}

std::string statusToString(testsgen::TestStatus status) {
    static const std::map<testsgen::TestStatus, std::string> description{
        { testsgen::TestStatus::TEST_PASSED, "PASSED" },
        { testsgen::TestStatus::TEST_FAILED, "FAILED" },
        { testsgen::TestStatus::TEST_DEATH, "DEATH" },
        { testsgen::TestStatus::TEST_INTERRUPTED, "INTERRUPTED" }
    };
    auto it = description.find(status);
    return it == description.end() ? "UNKNOWN" : it->second;
}

void CLICoverageAndResultsWriter::writeResponse(const utbot::ProjectContext &projectContext,
                                                const Coverage::TestResultMap &testsResultMap,
                                                const Coverage::CoverageMap &coverageMap,
                                                const nlohmann::json &totals,
                                                std::optional<std::string> errorMessage) {
    std::stringstream ss;

    ss << "Test results summary." << std::endl;
    for (const auto &[filepath, fileTestsResult] : testsResultMap) {
        ss << "==== Tests in " << filepath << std::endl;
        for (const auto &[testName, result] : fileTestsResult) {
            ss << "======== " << testName << " -> " << statusToString(result.status()) << std::endl;
        }
    }

    ss << "Coverage summary." << std::endl;
    for (const auto &[filepath, coverage] : coverageMap) {
        ss << "==== Coverage in " << filepath << std::endl;
        ss << "======== covered lines: ";
        for (const auto &sourceLine : coverage.fullCoverageLines) {
            ss << sourceLine.line << " ";
        }
        ss << std::endl;
        ss << "======== uncovered line ranges: ";
        for (const auto &sourceLine : coverage.noCoverageLines) {
            ss << sourceLine.line << " ";
        }
        ss << std::endl;
    }
    ss << "Totals:\n";
    ss << totals;
    fs::path resultsFilePath = Paths::getUTBotReportDir(projectContext) / "tests-result.log";
    FileSystemUtils::writeToFile(resultsFilePath, ss.str());
    LOG_S(INFO) << ss.str();
}
