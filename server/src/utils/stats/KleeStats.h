#ifndef UTBOTCPP_KLEESTATS_H
#define UTBOTCPP_KLEESTATS_H

#include <chrono>
#include <istream>

namespace StatsUtils {
    class KleeStats {
    public:
        KleeStats() : kleeTime(0), solverTime(0), resolutionTime(0) {}
        KleeStats(std::chrono::milliseconds kleeTime, std::chrono::milliseconds solverTime,
                  std::chrono::milliseconds resolutionTime) :
                kleeTime(kleeTime), solverTime(solverTime), resolutionTime(resolutionTime) {}

        explicit KleeStats(std::istream &kleeStatsReport);

        KleeStats &operator+=(const KleeStats &other);

        KleeStats operator+(KleeStats other) const;

        [[nodiscard]] std::chrono::milliseconds getKleeTime() const {
            return kleeTime;
        }

        [[nodiscard]] std::chrono::milliseconds getSolverTime() const {
            return solverTime;
        }

        [[nodiscard]] std::chrono::milliseconds getResolutionTime() const {
            return resolutionTime;
        }

    private:
        std::chrono::milliseconds kleeTime;
        std::chrono::milliseconds solverTime;
        std::chrono::milliseconds resolutionTime;
    };
}

#endif //UTBOTCPP_KLEESTATS_H
