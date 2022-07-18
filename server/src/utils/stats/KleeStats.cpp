#include "KleeStats.h"

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
}
