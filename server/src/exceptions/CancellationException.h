/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_CancellationException_H
#define UNITTESTBOT_CancellationException_H

#include <exception>

struct CancellationException : public std::exception {
    explicit CancellationException() = default;

    [[nodiscard]] const char *what() const noexcept override {
        return "Request has been cancelled.";
    }
};

#endif //UNITTESTBOT_CancellationException_H
