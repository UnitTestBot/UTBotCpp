/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_LOGUTIL_H
#define UNITTESTBOT_LOGUTIL_H
#include "TimeUtils.h"

#include "utils/path/FileSystemPath.h"

class Server;

namespace LogUtils {
    static const std::string TEST_CLIENT = "UnitTestBot";
    static const std::string UNNAMED_CLIENT = "unnamedClient";
    static const std::string NO_PROJECT = "noProject";
    static const std::string LOG_CHANNELS_WATCHER = "logChannelsWatcher";

    bool isMaxVerbosity();

    /**
     * @brief Writes log message to file.
     * @param log Log message.
     * @param projectName Name of directory where log file will be written.
     * @param stage Name of the stage of test generation pipeline.
     * @return Path to the log file.
     */
    fs::path writeLog(const std::string &log, const std::string &projectName, const std::string &stage);

    std::string errnoMessage();

    bool logChannelsWatcher(Server &server);
}

#endif //UNITTESTBOT_LOGUTIL_H
