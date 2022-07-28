#include "KleeStats.h"

#include <vector>

#include "utils/stats/CSVReader.h"

namespace StatsUtils {
    KleeStats &KleeStats::operator+=(const KleeStats &other) {
        kleeTime += other.kleeTime;
        solverTime += other.solverTime;
        resolutionTime += other.resolutionTime;
        return *this;
    }

    KleeStats KleeStats::operator+(KleeStats other) const {
        other += *this;
        return other;
    }

    KleeStats::KleeStats(std::istream &kleeStatsReport) {
        StatsUtils::CSVTable parsedCSV = StatsUtils::readCSV(kleeStatsReport, ',');
        std::vector<std::string> keys = {"Time(s)", "TSolver(s)", "TResolve(s)"};
        std::vector<std::chrono::milliseconds> timeValues;
        for (const auto &key: keys) {
            int totalTime = 0;
            if (CollectionUtils::containsKey(parsedCSV, key)) {
                totalTime = 1000 * std::stof(parsedCSV[key].back());
            } else {
                LOG_S(WARNING) << StringUtils::stringFormat("Key %s not found in klee-stats report", key);
            }
            timeValues.emplace_back(totalTime);
        }
        kleeTime = timeValues[0];
        solverTime = timeValues[1];
        resolutionTime = timeValues[2];
    }
}
