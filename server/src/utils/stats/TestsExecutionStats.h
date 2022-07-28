#ifndef UTBOTCPP_TESTSEXECUTIONSTATS_H
#define UTBOTCPP_TESTSEXECUTIONSTATS_H

#include <chrono>
#include <unordered_map>
#include <utility>

#include "protobuf/testgen.grpc.pb.h"
#include "coverage/Coverage.h"
#include "ProjectContext.h"
#include "loguru.h"
#include "utils/StringUtils.h"
#include "utils/stats/FileStatsMap.h"

namespace StatsUtils {

    class TestsExecutionStats {
    public:
        TestsExecutionStats() = default;

        TestsExecutionStats(const Coverage::FileTestsResult &testsResult, const Coverage::FileCoverage &coverage);

        TestsExecutionStats &operator+=(const TestsExecutionStats &other);

        TestsExecutionStats operator+(TestsExecutionStats other) const;

        [[nodiscard]] std::vector<std::string> toStrings() const;

    private:
        // Time stats
        google::protobuf::Duration totalExecutionTime;

        // Test runs stats
        uint32_t totalTestsNum = 0;
        std::unordered_map<testsgen::TestStatus, uint32_t> testsWithStatusNum = {};

        // Coverage
        uint32_t fullCoverageLinesNum = 0;
        uint32_t partialCoverageLinesNum = 0;
        uint32_t noCoverageLinesNum = 0;
    };

    class TestsExecutionStatsFileMap : public FileStatsMap<TestsExecutionStats> {
    public:
        TestsExecutionStatsFileMap(const utbot::ProjectContext &projectContext,
                                   const Coverage::TestResultMap &testsResultMap,
                                   const Coverage::CoverageMap &coverageMap);

        std::vector<std::string> getHeader() override;
    };
}

#endif //UTBOTCPP_TESTSEXECUTIONSTATS_H
