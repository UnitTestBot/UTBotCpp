/*
* Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
*/

#ifndef UNITTESTBOT_ERRORMODE_H
#define UNITTESTBOT_ERRORMODE_H
namespace ErrorMode {

    enum class ErrorMode {
        FAILING = 0,
        PASSING_IN_TARGET_ONLY = 1,
        PASSING = 2
    };

} // ErrorMode
#endif // UNITTESTBOT_ERRORMODE_H
