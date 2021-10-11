/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "GTestLogger.h"

#include "loguru.h"

void GTestLogger::log(std::string_view message) {
    LOG_S(INFO) << message;
}

const char* GTestLogger::fileName() {
    return __FILE__;
}
