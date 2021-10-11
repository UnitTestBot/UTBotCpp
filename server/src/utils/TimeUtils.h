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

    using std::string;

    typedef std::chrono::time_point<std::chrono::_V2::system_clock> systemClockTimePoint;

    static const auto DAY_DURATION = 86400s;

    //Idleness period after which log channels will be released.
    static const auto IDLE_TIMEOUT = 1h;

    template <typename TP>
    std::time_t to_time_t(TP tp) {
        using namespace std::chrono;
        auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now() + system_clock::now());
        return system_clock::to_time_t(sctp);
    }

    systemClockTimePoint now();

    bool isOutdatedTimestamp(const systemClockTimePoint& now, const systemClockTimePoint& old);

    bool isOutdatedTimestamp(const systemClockTimePoint& old);

    systemClockTimePoint convertFileToSystemClock(const fs::file_time_type& fTime);

    string getDate();
}

#endif //UNITTESTBOT_TIMEUTIL_H
