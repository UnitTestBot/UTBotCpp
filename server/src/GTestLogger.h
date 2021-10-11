/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_GTESTLOGGER_H
#define UNITTESTBOT_GTESTLOGGER_H

#include <string>

class GTestLogger {
public:
    static void log(std::string_view message);

    static const char* fileName();
};


#endif //UNITTESTBOT_GTESTLOGGER_H
