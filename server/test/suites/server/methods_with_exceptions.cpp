/*
* Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
*/

#include "exception.h"
#include <exception>
#include <stdexcept>

int basicException(int x) {
    if (x < 5) {
        throw std::exception();
    }
    return 42;
}

int customExceptionFromStd(int x) {
    if (x % 3 == 1) {
        throw std::exception();
    } else if (x % 3 == 2) {
        throw std::runtime_error("Error");
    } else {
        return 42;
    }
}

int customExceptionDerivedFromStdException(int x) {
    if (x > 42) {
        throw MyException();
    }
    return 42;
}

int customExceptionDerivedFromRuntimeException(int x) {
    if (x % 3 == 0) {
        throw MyRuntimeException();
    }
    return 42;
}
