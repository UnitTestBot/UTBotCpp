/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "ExecutionProcessException.h"

ExecutionProcessException::ExecutionProcessException(std::string cmd, fs::path logFilePath) : cmd(std::move(cmd)), logFilePath(std::move(logFilePath)) {}

const char *ExecutionProcessException::what() const noexcept {
    return cmd.c_str();
}

std::string ExecutionProcessException::getLogFilePath() const noexcept {
    return logFilePath;
}
