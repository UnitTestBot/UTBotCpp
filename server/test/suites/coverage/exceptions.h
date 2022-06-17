/*
* Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_EXCEPTION_H
#define UNITTESTBOT_EXCEPTION_H
#include <exception>

struct MyException : std::exception {
    MyException() : std::exception() {}
};

#endif // UNITTESTBOT_EXCEPTION_H
