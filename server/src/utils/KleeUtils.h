#ifndef CORE_KLEEUTIL_H
#define CORE_KLEEUTIL_H

#include <grpcpp/server_context.h>
#include <string>
#include <string_view>

#include "Tests.h"

namespace KleeUtils {
    static inline const std::string TEMP_VARIABLE_NAME = "utbot_tmp";
    static inline const std::string RESULT_VARIABLE_NAME = "utbot_result";
    static inline const std::string NOT_NULL_VARIABLE_NAME = "utbot_return_not_null";

    static inline const std::string STDIN_READ_NAME = "stdin_read";
    static inline const std::string STDIN_NAME = "stdin";

    std::string getRenamedOperator(std::string_view methodName);

    std::string entryPointFunction(const tests::Tests &tests,
                                   const std::string &methodName,
                                   bool needToMangle = false,
                                   bool isWrapped = false);

    std::string postSymbolicVariable(const std::string &variableName);

    std::string processNumberOption();
}

#endif // CORE_KLEEUTIL_H
