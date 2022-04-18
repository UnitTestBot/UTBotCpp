/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_CLIUTILS_H
#define UNITTESTBOT_CLIUTILS_H

#include "Server.h"
#include "utils/path/FileSystemPath.h"

#include "loguru.h"

#include <CLI11.hpp>
#include <rang.hpp>
#include <string>

namespace CLIUtils {
    void setOptPath(const std::string &optPath, fs::path &var);

    void setupLogger(const std::string &logPath,
                     const loguru::NamedVerbosity &verbosity,
                     bool threadView = true);

    void setupLogger(int argc, char **argv, bool threadView = true);

    void parse(int argc, char **argv, CLI::App &app);

    loguru::NamedVerbosity getVerbosityLevelFromName(const char *name);

    char *getCmdOption(char **begin, char **end, const std::string &option);

    void setOptPath(int argc, char **argv, const std::string &option, fs::path &var);
}

#endif // UNITTESTBOT_CLIUTILS_H
