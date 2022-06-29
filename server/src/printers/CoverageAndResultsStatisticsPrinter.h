#ifndef UNITTESTBOT_COVERAGEANDRESULTSSTATISTICSPRINTER_H
#define UNITTESTBOT_COVERAGEANDRESULTSSTATISTICSPRINTER_H

#include <chrono>
#include <unordered_map>
#include <utility>
#include <protobuf/testgen.grpc.pb.h>
#include <ProjectContext.h>
#include "coverage/Coverage.h"
#include "loguru.h"

namespace printer {
    class FileCoverageAndResultsStatistics {
    public:
        FileCoverageAndResultsStatistics() = default;
        FileCoverageAndResultsStatistics(const Coverage::FileTestsResult &testsResult, const Coverage::FileCoverage &coverage);

        // Time statistics
        google::protobuf::Duration totalExecutionTime;

        // Test runs statistics
        uint32_t totalTestsNum = 0;
        std::unordered_map<testsgen::TestStatus, uint32_t> testsWithStatusNum = {};

        // Coverage
        uint32_t fullCoverageLinesNum = 0;
        uint32_t partialCoverageLinesNum = 0;
        uint32_t noCoverageLinesNum = 0;

        friend std::ostream& operator<<(std::ostream &os, const FileCoverageAndResultsStatistics &statistics);
        FileCoverageAndResultsStatistics& operator+=(const FileCoverageAndResultsStatistics &other);
        FileCoverageAndResultsStatistics operator+(FileCoverageAndResultsStatistics other) const;
    };

    std::ostream& operator<<(std::ostream &os, const FileCoverageAndResultsStatistics &statistics);

    class CoverageAndResultsStatisticsPrinter : CollectionUtils::MapFileTo<FileCoverageAndResultsStatistics> {
    public:
        CoverageAndResultsStatisticsPrinter() = default;
        std::stringstream write(const utbot::ProjectContext &projectContext,
                                const Coverage::TestResultMap &testsResultMap,
                                const Coverage::CoverageMap &coverageMap);

    private:
        fs::path resultsDirectory;
    };
}

#endif //UNITTESTBOT_COVERAGEANDRESULTSSTATISTICSPRINTER_H
