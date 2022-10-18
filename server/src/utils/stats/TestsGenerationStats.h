#ifndef UTBOTCPP_TESTSGENERATIONSTATS_H
#define UTBOTCPP_TESTSGENERATIONSTATS_H

#include <map>
#include <utility>

#include "utils/stats/KleeStats.h"
#include "utils/CollectionUtils.h"
#include "utils/stats/FileStatsMap.h"
#include "utils/StringUtils.h"
#include "Tests.h"

namespace StatsUtils {
    class TestsGenerationStats {
    public:
        TestsGenerationStats() = default;

        TestsGenerationStats(const KleeStats &kleeStats, const tests::Tests &tests);

        KleeStats kleeStats;
        // map test-suite name to number of generated tests in this suite
        std::map<std::string, uint32_t> numTestsInSuite;
        uint32_t numCoveredFunctions = 0;
        uint32_t numFunctions = 0;

        TestsGenerationStats &operator+=(const TestsGenerationStats &other);

        TestsGenerationStats operator+(TestsGenerationStats other) const;

        [[nodiscard]] std::vector<std::string> toStrings() const;
    };

    class TestsGenerationStatsFileMap : public FileStatsMap<TestsGenerationStats> {
    public:
        TestsGenerationStatsFileMap() = delete;

        TestsGenerationStatsFileMap(utbot::ProjectContext projectContext, std::chrono::milliseconds preprocessingTime)
                : FileStatsMap(std::move(projectContext)), preprocessingTime(preprocessingTime) {}

        void addFileStats(const KleeStats &kleeStats, const tests::Tests &tests);

        std::vector<std::string> getHeader() override;

        std::vector<std::string> getTotalStrings() override;

    private:
        std::chrono::milliseconds preprocessingTime;
    };
}

#endif //UTBOTCPP_TESTSGENERATIONSTATS_H
