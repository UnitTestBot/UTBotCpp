#ifndef UTBOTCPP_STUBUTILS_H
#define UTBOTCPP_STUBUTILS_H

#include <string>
#include <optional>

namespace StubsUtils {
    std::string getFunctionPointerStubName(const std::optional<std::string> &scopeName,
                                           const std::string &methodName,
                                           const std::string &paramName,
                                           bool omitSuffix);

    std::string getFunctionPointerAsStructFieldStubName(const std::string &structName,
                                                        const std::string &fieldName,
                                                        bool omitSuffix);

    std::string getStubSymbolicVarName(const std::string &methodName, const std::string &parentMethodName);

    std::string tryGetMethodNameFromStubSymbolic(const std::string &symbolicName);

}

#endif //UTBOTCPP_STUBUTILS_H
