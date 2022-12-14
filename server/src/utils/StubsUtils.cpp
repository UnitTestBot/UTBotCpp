#include "StubsUtils.h"
#include "PrinterUtils.h"

namespace StubsUtils {
    std::string getFunctionPointerStubName(const std::optional<std::string> &scopeName,
                                           const std::string &methodName,
                                           const std::string &paramName,
                                           bool omitSuffix) {
        std::string stubName = "*" + scopeName.value_or("") + "_" + methodName;
        if (!omitSuffix) {
            stubName += "_" + paramName + "_stub";
        } else {
            stubName = stubName.substr(1);
        }
        return stubName;
    }

    std::string parseParamNameFromFunctionPointerStubName(const std::string &functionPointerStubName,
                                                          const std::string &methodName) {
        return functionPointerStubName.substr(methodName.length() + 1, functionPointerStubName.length());
    }

    std::string getStubSymbolicVarName(const std::string &methodName) {
        return methodName + PrinterUtils::KLEE_SYMBOLIC_SUFFIX;
    }

    bool isStubSymbolicVarName(const std::string &name) {
        return StringUtils::endsWith(name, PrinterUtils::KLEE_SYMBOLIC_SUFFIX);
    }

    std::string parseMethodNameFromStubSymbolicVarName(const std::string &symbolicVarName) {
        return symbolicVarName.substr(0, symbolicVarName.length() - PrinterUtils::KLEE_SYMBOLIC_SUFFIX.length());
    }

    std::string getFunctionPointerAsStructFieldStubName(const std::string &structName,
                                                        const std::string &fieldName,
                                                        bool omitSuffix) {
        std::string stubName = "*" + structName;
        if (!omitSuffix) {
            stubName += "_" + fieldName + "_stub";
        } else {
            stubName = stubName.substr(1);
        }
        return stubName;
    }
}
