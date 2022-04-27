/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "LogUtils.h"

#include "FileSystemUtils.h"
#include "Paths.h"
#include "Server.h"
#include "StringUtils.h"

#include "loguru.h"

#include <thread>

namespace LogUtils {
    bool isMaxVerbosity() {
        return loguru::g_stderr_verbosity == loguru::Verbosity_MAX;
    }

    fs::path writeLog(const string &log, const string &projectName, const string &stage) {
        fs::path compileLogPath;
        if (projectName.empty()) {
            LOG_S(WARNING) << "writeLog: projectName is empty. Logs will be written in folder " + NO_PROJECT;
            compileLogPath = Paths::getExecLogPath(NO_PROJECT);
        } else {
            compileLogPath = Paths::getExecLogPath(projectName);
        }
        auto logFilename = TimeUtils::getDate() + ".log";
        compileLogPath /= logFilename;
        FileSystemUtils::writeToFile(compileLogPath, log);
        return compileLogPath;
    }

    std::string errnoMessage() {
        return StringUtils::stringFormat("ERRNO: %s", strerror(errno));
    }

    bool logChannelsWatcher(Server &server) {
        loguru::set_thread_name(LOG_CHANNELS_WATCHER.c_str());
        auto &service = server.testsService;
        while (true) {
            if (server.logChannelsWatcherCancellationToken) {
                return true;
            }
            std::this_thread::sleep_for(TimeUtils::IDLE_TIMEOUT);
            const std::lock_guard<std::mutex> lock(server.testsService.logChannelOperationsMutex);
            auto now = TimeUtils::now();
            std::vector <std::string> outdatedClients;
            for (const auto& [client, timestamp] : service.linkedWithClient) {
                if (TimeUtils::isOutdatedTimestamp(now, timestamp)) {
                    LOG_S(INFO) << "Client " << client << " is outdated.";
                    outdatedClients.emplace_back(client);
                }
            }
            for (const auto& client : outdatedClients) {
                service.linkedWithClient.erase(client);
                service.openedChannel[client].store(false, std::memory_order_release);
                service.openedGTestChannel[client].store(false, std::memory_order_release);
            }
        }
    }
}
