#include "TestsGenerationStats.h"

namespace StatsUtils {

    TestsGenerationStats::TestsGenerationStats(const KleeStats &kleeStats, const tests::Tests &tests) :
            kleeStats(kleeStats) {
        numTestsInSuite["regression"] = tests.regressionMethodsNumber;
        numTestsInSuite["error"] = tests.errorMethodsNumber;
        numCoveredFunctions = 0;
        for (const auto &[methodName, methodDescription] : tests.methods) {
            if (!methodDescription.testCases.empty()) {
                numCoveredFunctions += 1;
            }
        }
        numFunctions = tests.methods.size();
    }

    TestsGenerationStats &TestsGenerationStats::operator+=(const TestsGenerationStats &other) {
        kleeStats += other.kleeStats;
        for (const auto &[testSuite, numTests]: other.numTestsInSuite) {
            numTestsInSuite[testSuite] += numTests;
        }
        numCoveredFunctions += other.numCoveredFunctions;
        numFunctions += other.numFunctions;
        return *this;
    }

    TestsGenerationStats TestsGenerationStats::operator+(TestsGenerationStats other) const {
        other += *this;
        return other;
    }

    std::vector<std::string> TestsGenerationStats::toStrings() const {
        std::vector<std::string> out;
        // Klee-stats
        {
            out.push_back(StringUtils::stringFormat("%.2f", kleeStats.getKleeTime().count() / 1000.0));
            out.push_back(StringUtils::stringFormat("%.2f", kleeStats.getSolverTime().count() / 1000.0));
            out.push_back(StringUtils::stringFormat("%.2f", kleeStats.getResolutionTime().count() / 1000.0));
        }
        // Tests in different suites
        {
            for (const auto &suite : {"regression", "error"}) {
                out.push_back(StringUtils::stringFormat("%d", CollectionUtils::getOrDefault(numTestsInSuite,
                                                                                            (std::string)suite, 0u)));
            }
        }
        // Covered functions
        {
            out.push_back(StringUtils::stringFormat("%d", numCoveredFunctions));
            out.push_back(StringUtils::stringFormat("%d", numFunctions));
        }
        return out;
    }

    std::vector<std::string> TestsGenerationStatsFileMap::getHeader() {
        return {"Klee Time (s)", "Solver Time (s)", "Resolution Time (s)", "Regression Tests Generated",
                "Error Tests Generated", "Covered Functions", "Total functions"};
    }

    std::vector<std::string> TestsGenerationStatsFileMap::getTotalStrings() {
        std::vector<std::string> out = getTotal().toStrings();
        out[0] += StringUtils::stringFormat(" (preprocessing +%.2f)", preprocessingTime.count() / 1000.0);
        return out;
    }

    void TestsGenerationStatsFileMap::addFileStats(const KleeStats &kleeStats, const tests::Tests &tests) {
        statsMap[tests.sourceFilePath] = TestsGenerationStats(kleeStats, tests);
    }
}
