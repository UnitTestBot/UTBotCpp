/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_COMPILATIONDATABASEEXCEPTION_H
#define UNITTESTBOT_COMPILATIONDATABASEEXCEPTION_H

#include "BaseException.h"

struct CompilationDatabaseException : public BaseException {
    explicit CompilationDatabaseException(std::string message) : BaseException(std::move(message)) {
    }
};

#endif // UNITTESTBOT_COMPILATIONDATABASEEXCEPTION_H
