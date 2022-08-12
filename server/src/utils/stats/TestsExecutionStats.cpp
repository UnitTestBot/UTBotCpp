#include "Paths.h"
#include "TestsExecutionStats.h"
#include "utils/StringUtils.h"

namespace StatsUtils {
    TestsExecutionStats::TestsExecutionStats(const Coverage::FileTestsResult &testsResult,
                                             const Coverage::FileCoverage &coverage) {
        totalTestsNum = 0;
        totalExecutionTime = google::protobuf::Duration();
        for (const auto &[_, testResult]: testsResult) {
            totalTestsNum++;
            testsWithStatusNum[testResult.status()]++;
            totalExecutionTime += testResult.executiontime();
        }
        fullCoverageLinesNum = coverage.fullCoverageLines.size();
        partialCoverageLinesNum = coverage.partialCoverageLines.size();
        noCoverageLinesNum = coverage.noCoverageLines.size();
    }

    TestsExecutionStats &
    TestsExecutionStats::operator+=(const TestsExecutionStats &other) {
        totalExecutionTime += other.totalExecutionTime;
        totalTestsNum += other.totalTestsNum;
        for (const auto &[status, testsNum]: other.testsWithStatusNum) {
            testsWithStatusNum[status] += testsNum;
        }
        fullCoverageLinesNum += other.fullCoverageLinesNum;
        partialCoverageLinesNum += other.partialCoverageLinesNum;
        noCoverageLinesNum += other.noCoverageLinesNum;
        return *this;
    }

    TestsExecutionStats
    TestsExecutionStats::operator+(TestsExecutionStats other) const {
        other += *this;
        return other;
    }

    std::vector<std::string> TestsExecutionStats::toStrings() const {
        std::vector<std::string> out;
        // TIME
        {
            // total tests execution time
            std::string totalExecutionTimeStr = google::protobuf::util::TimeUtil::ToString(totalExecutionTime);
            out.push_back(totalExecutionTimeStr.substr(0, totalExecutionTimeStr.size() - 1));
        }

        // TEST STATUS
        {
            // total number of tests
            out.push_back(StringUtils::stringFormat("%u", totalTestsNum));
            for (const auto &testStatus: {testsgen::TestStatus::TEST_PASSED, testsgen::TestStatus::TEST_FAILED,
                                          testsgen::TestStatus::TEST_DEATH, testsgen::TestStatus::TEST_INTERRUPTED}) {
                uint32_t testsNum = CollectionUtils::getOrDefault(testsWithStatusNum, testStatus, 0u);
                out.push_back(StringUtils::stringFormat("%u", testsNum));
            }
        }

        // COVERAGE
        {
            uint32_t coveredLinesNum = fullCoverageLinesNum + partialCoverageLinesNum;
            uint32_t totalLinesNum = coveredLinesNum + noCoverageLinesNum;
            double lineCoverageRatio = 0;
            if (totalLinesNum != 0) {
                lineCoverageRatio = 100.0 * coveredLinesNum / totalLinesNum;
            }
            // total number of lines
            out.push_back(StringUtils::stringFormat("%u", totalLinesNum));
            // number of covered lines
            out.push_back(StringUtils::stringFormat("%u", coveredLinesNum));
            // line coverage ratio
            out.push_back(StringUtils::stringFormat("%f", lineCoverageRatio));
        }
        return out;
    }

    std::vector<std::string> TestsExecutionStatsFileMap::getHeader() {
        return {"Google Test Execution Time (s)",
                "Total Tests Number", "Passed Tests Number", "Failed Tests Number", "Death Tests Number",
                "Interrupted Tests Number",
                "Total Lines Number", "Covered Lines Number", "Line Coverage Ratio (%)"};
    }

    TestsExecutionStatsFileMap::TestsExecutionStatsFileMap(const utbot::ProjectContext &projectContext,
                                                           const Coverage::TestResultMap &testsResultMap,
                                                           const Coverage::CoverageMap &coverageMap)
            : FileStatsMap(projectContext) {
        for (auto const &[testPath, testsResult]: testsResultMap) {
            fs::path sourcePath = Paths::testPathToSourcePath(projectContext, testPath);
            Coverage::FileCoverage fileCoverage = CollectionUtils::getOrDefault(coverageMap,
                                                                                sourcePath,
                                                                                Coverage::FileCoverage());
            statsMap[sourcePath] = TestsExecutionStats(testsResult, fileCoverage);
        }
    }
}
