/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_RESULT_H
#define UNITTESTBOT_RESULT_H

#include <string>
#include <variant>

template<typename T, typename ErrorType=std::string>
class Result {
public:
    Result(T t) : result(std::optional<T>(t)) {}

    Result(ErrorType error) : result(std::optional<ErrorType>(error)) {}

    std::optional<T> getOpt() {
        return std::get<std::optional<T>>(result);
    }

    bool isSuccess() {
        return std::get_if<std::optional<T>>(&result);
    }

    std::optional<ErrorType> getError() {
        return std::get<std::optional<ErrorType>>(result);
    }

private:
    std::variant<std::optional<T>, std::optional<ErrorType>> result;
};

#endif //UNITTESTBOT_RESULT_H
