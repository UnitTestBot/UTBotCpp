/*
* Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_EXCEPTION_H
#define UNITTESTBOT_EXCEPTION_H
#include <exception>
#include <stdexcept>

struct MyException : std::exception {};

struct MyRuntimeException : std::runtime_error {
    MyRuntimeException() : std::runtime_error("My Runtime Error") {}
};

#endif // UNITTESTBOT_EXCEPTION_H
