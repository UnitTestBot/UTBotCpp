/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_EXECUTIONPROCESSEXCEPTION_H
#define UNITTESTBOT_EXECUTIONPROCESSEXCEPTION_H

#include <exception>
#include <string>
#include <utility>
#include "utils/path/FileSystemPath.h"

using std::string;

struct ExecutionProcessException : public std::exception {
    explicit ExecutionProcessException(string cmd, std::optional<fs::path> logFilePath);

    [[nodiscard]] const char *what() const noexcept override;

    [[nodiscard]] string getLogFilePath() const noexcept;

private:
    string cmd;
    std::optional<fs::path> logFilePath;
};

#endif //UNITTESTBOT_EXECUTIONPROCESSEXCEPTION_H
