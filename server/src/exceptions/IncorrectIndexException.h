/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_INCORRECTINDEXEXCEPTION_H
#define UNITTESTBOT_INCORRECTINDEXEXCEPTION_H

#include "BaseException.h"

struct IncorrectIndexException : public BaseException {
    explicit IncorrectIndexException(std::string message) : BaseException(message) {}
};

#endif //UNITTESTBOT_INCORRECTINDEXEXCEPTION_H
