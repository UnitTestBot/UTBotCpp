/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */


#include "CLICoverageAndResultsWriter.h"

#include "loguru.h"

#include <fstream>
#include <utils/FileSystemUtils.h>
#include <utils/TimeUtils.h>

CLICoverageAndResultsWriter::CLICoverageAndResultsWriter(const fs::path &resultsDirectory)
    : resultsDirectory(resultsDirectory), CoverageAndResultsWriter(nullptr) {
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

void CLICoverageAndResultsWriter::writeResponse(const Coverage::TestStatusMap &testsStatusMap,
                                                const Coverage::CoverageMap &coverageMap,
                                                const nlohmann::json &totals,
                                                std::optional<string> errorMessage) {
    std::stringstream ss;

    ss << "Test results summary." << std::endl;
    for (const auto &[filepath, fileTestsStatus] : testsStatusMap) {
        ss << "==== Tests in " << filepath << std::endl;
        for (const auto &[testName, status] : fileTestsStatus) {
            ss << "======== " << testName << " -> " << statusToString(status) << std::endl;
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
    fs::path resultsFilePath = resultsDirectory / (TimeUtils::getDate() + ".log");
    FileSystemUtils::writeToFile(resultsFilePath, ss.str());
    LOG_S(INFO) << ss.str();
}
