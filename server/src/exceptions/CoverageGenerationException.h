/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_COVERAGEGENERATIONEXCEPTION_H
#define UNITTESTBOT_COVERAGEGENERATIONEXCEPTION_H

#include "BaseException.h"

struct CoverageGenerationException : public BaseException {
    explicit CoverageGenerationException(std::string message) : BaseException(std::move(message)) {
    }

    explicit CoverageGenerationException(const char *message) : BaseException(message) {
    }
};

#endif // UNITTESTBOT_COVERAGEGENERATIONEXCEPTION_H
