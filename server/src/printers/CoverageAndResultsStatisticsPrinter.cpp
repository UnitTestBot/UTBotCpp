#include <utils/FileSystemUtils.h>

#include "CoverageAndResultsStatisticsPrinter.h"
#include "utils/StringUtils.h"
#include "utils/CollectionUtils.h"
#include "Paths.h"
#include <google/protobuf/util/time_util.h>

namespace printer {
    std::ostream &operator<<(std::ostream &os, const FileCoverageAndResultsStatistics &statistics) {
        // total tests execution file
        std::string totalExecutionTimeStr = google::protobuf::util::TimeUtil::ToString(statistics.totalExecutionTime);
        os << totalExecutionTimeStr.substr(0, totalExecutionTimeStr.size() - 1) << ',';
        // total number of tests
        os << statistics.totalTestsNum << ",";
        // number of passed tests
        os << CollectionUtils::getOrDefault(statistics.testsWithStatusNum, testsgen::TestStatus::TEST_PASSED, 0u)
           << ',';
        // number of failed tests
        os << CollectionUtils::getOrDefault(statistics.testsWithStatusNum, testsgen::TestStatus::TEST_FAILED, 0u)
           << ',';
        // number of death tests
        os << CollectionUtils::getOrDefault(statistics.testsWithStatusNum, testsgen::TestStatus::TEST_DEATH, 0u) << ',';
        // number of interrupted tests
        os << CollectionUtils::getOrDefault(statistics.testsWithStatusNum, testsgen::TestStatus::TEST_INTERRUPTED, 0u)
           << ',';

        uint32_t coveredLinesNum = statistics.fullCoverageLinesNum + statistics.partialCoverageLinesNum;
        uint32_t totalLinesNum = coveredLinesNum + statistics.noCoverageLinesNum;
        double lineCoverageRatio = 0;
        if (totalLinesNum != 0) {
            lineCoverageRatio = 100.0 * coveredLinesNum / totalLinesNum;
        }
        // total number of lines and number of covered lines
        os << totalLinesNum << ',' << coveredLinesNum << ',';
        // line coverage ratio
        os << std::fixed << lineCoverageRatio;
        return os;
    }

    std::stringstream CoverageAndResultsStatisticsPrinter::write(const utbot::ProjectContext &projectContext,
                                                                 const Coverage::TestResultMap &testsResultMap,
                                                                 const Coverage::CoverageMap &coverageMap) {
        for (auto const &[testPath, testsResult]: testsResultMap) {
            fs::path sourcePath = Paths::testPathToSourcePath(projectContext, testPath);
            Coverage::FileCoverage fileCoverage = CollectionUtils::getOrDefault(coverageMap,
                                                                                sourcePath,
                                                                                Coverage::FileCoverage());
            insert({sourcePath, FileCoverageAndResultsStatistics(testsResult, fileCoverage)});
        }
        std::vector<std::string> metricNames = {
                "Filename", "Google Test Execution Time (s)",
                "Total Tests Number", "Passed Tests Number", "Failed Tests Number", "Death Tests Number",
                "Interrupted Tests Number",
                "Total Lines Number", "Covered Lines Number", "Line Coverage Ratio (%)"
        };
        std::string header = StringUtils::joinWith(metricNames, ",");
        std::stringstream ss;
        ss << header << '\n';

        FileCoverageAndResultsStatistics total;
        for (auto const &[sourcePath, statistics]: *this) {
            total += statistics;
            ss << fs::relative(sourcePath, projectContext.projectPath).string() << ",";
            ss << statistics << '\n';
        }
        ss << "Total," << total << '\n';
        return ss;
    }

    FileCoverageAndResultsStatistics::FileCoverageAndResultsStatistics(
            const Coverage::FileTestsResult &testsResult,
            const Coverage::FileCoverage &fileCoverage) {
        totalTestsNum = 0;
        totalExecutionTime = google::protobuf::Duration();
        for (const auto &[_, testResult]: testsResult) {
            totalTestsNum++;
            testsWithStatusNum[testResult.status()]++;
            totalExecutionTime += testResult.executiontime();
        }
        fullCoverageLinesNum = fileCoverage.fullCoverageLines.size();
        partialCoverageLinesNum = fileCoverage.partialCoverageLines.size();
        noCoverageLinesNum = fileCoverage.noCoverageLines.size();
    }

    FileCoverageAndResultsStatistics &
    FileCoverageAndResultsStatistics::operator+=(const FileCoverageAndResultsStatistics &other) {
        totalExecutionTime += other.totalExecutionTime;
        totalTestsNum += other.totalTestsNum;
        for (const auto &[status, testsNum] : other.testsWithStatusNum) {
            testsWithStatusNum[status] += testsNum;
        }
        fullCoverageLinesNum += other.fullCoverageLinesNum;
        partialCoverageLinesNum += other.partialCoverageLinesNum;
        noCoverageLinesNum += other.noCoverageLinesNum;
        return *this;
    }

    FileCoverageAndResultsStatistics
    FileCoverageAndResultsStatistics::operator+(FileCoverageAndResultsStatistics other) const {
        other += *this;
        return other;
    }
}
