/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_LLVMEXCEPTION_H
#define UNITTESTBOT_LLVMEXCEPTION_H

#include "BaseException.h"

struct LLVMException : public BaseException {
    explicit LLVMException(std::string message) : BaseException(std::move(message)) {
    }
};

#endif //UNITTESTBOT_LLVMEXCEPTION_H
