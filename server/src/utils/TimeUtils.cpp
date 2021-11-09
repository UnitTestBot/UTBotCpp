/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "TimeUtils.h"
#include "StringUtils.h"

#include <algorithm>

namespace TimeUtils {
    using std::string;

    string getDate() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::ostringstream ss;
        ss << std::put_time(gmtime(&time_t), "%FT%T");
        ss << std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;
        std::string str = ss.str();
        return StringUtils::replaceAll(str, ':', '-');
    }

    systemClockTimePoint now() {
        return std::chrono::system_clock::now();
    }

    bool isOutdatedTimestamp(const systemClockTimePoint& now, const systemClockTimePoint& old) {
        return (now - old > TimeUtils::IDLE_TIMEOUT);
    }

    bool isOutdatedTimestamp(const systemClockTimePoint& old) {
        return (now() - old > TimeUtils::IDLE_TIMEOUT);
    }

    systemClockTimePoint convertFileToSystemClock(const fs::file_time_type &fTime) {
        return std::chrono::system_clock::from_time_t(to_time_t(fTime));
    }
}