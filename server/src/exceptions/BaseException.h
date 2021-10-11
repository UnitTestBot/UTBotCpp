/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_BASEEXCEPTION_H
#define UNITTESTBOT_BASEEXCEPTION_H

#include <string>

using std::string;

struct BaseException : public std::exception {
    explicit BaseException(string message) : message(std::move(message)) {
    }

    [[nodiscard]] virtual const char *what() const noexcept override {
        return message.c_str();
    }

    virtual ~BaseException() {
    }

protected:
    string message;
};

#endif // UNITTESTBOT_BASEEXCEPTION_H
