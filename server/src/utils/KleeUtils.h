/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef CORE_KLEEUTIL_H
#define CORE_KLEEUTIL_H

#include <grpcpp/server_context.h>
#include <string>
#include <string_view>

#include "Tests.h"

namespace KleeUtils {
    using std::string;

    static inline const std::string TEMP_VARIABLE_NAME = "utbot_tmp";
    static inline const std::string RESULT_VARIABLE_NAME = "utbot_result";
    static inline const std::string NOT_NULL_VARIABLE_NAME = "utbot_return_not_null";

    string getRenamedOperator(std::string_view methodName);

    string entryPointFunction(const tests::Tests &tests,
                              const std::string &methodName,
                              bool needToMangle = false);

    std::string postSymbolicVariable(const string& variableName);

    std::string processNumberOption();
}

#endif // CORE_KLEEUTIL_H
