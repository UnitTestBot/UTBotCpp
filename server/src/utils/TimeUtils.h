/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_TIMEUTIL_H
#define UNITTESTBOT_TIMEUTIL_H

#include <chrono>
#include <string>
#include "utils/path/FileSystemPath.h"

namespace TimeUtils {
    using namespace std::chrono_literals;

    typedef std::chrono::time_point<std::chrono::_V2::system_clock> systemClockTimePoint;

    static const auto DAY_DURATION = 86400s;

    //Idleness period after which log channels will be released.
    static const auto IDLE_TIMEOUT = 1h;

    template <typename TP>
    std::time_t to_time_t(TP tp) {
        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(tp - TP::clock::now() + std::chrono::system_clock::now());
        return std::chrono::system_clock::to_time_t(sctp);
    }

    systemClockTimePoint now();

    bool isOutdatedTimestamp(const systemClockTimePoint& now, const systemClockTimePoint& old);

    bool isOutdatedTimestamp(const systemClockTimePoint& old);

    systemClockTimePoint convertFileToSystemClock(const fs::file_time_type& fTime);

    std::string getDate();

    std::string getCurrentTimeStr();
}

#endif //UNITTESTBOT_TIMEUTIL_H
