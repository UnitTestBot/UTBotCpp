#include "StubsUtils.h"
#include "PrinterUtils.h"

namespace StubsUtils {
    std::string getFunctionPointerStubName(const std::optional<std::string> &scopeName,
                                           const std::string &methodName,
                                           const std::string &paramName,
                                           bool omitSuffix) {
        std::string stubName = "*" + scopeName.value_or("") + "_" + methodName;
        if (!omitSuffix) {
            stubName += "_" + paramName + Paths::STUB_SUFFIX;
        } else {
            stubName = stubName.substr(1);
        }
        StringUtils::replaceColon(stubName);
        return stubName;
    }

    std::string getFunctionPointerAsStructFieldStubName(const std::string &structName,
                                                        const std::string &fieldName,
                                                        bool omitSuffix) {
        std::string stubName = "*" + structName;
        if (!omitSuffix) {
            stubName += "_" + fieldName + Paths::STUB_SUFFIX;
        } else {
            stubName = stubName.substr(1);
        }
        StringUtils::replaceColon(stubName);
        return stubName;
    }

    std::string getStubSymbolicVarName(const std::string &methodName, const std::string &parentMethodName) {
        std::string stubName;
        if (!parentMethodName.empty()) {
            stubName = parentMethodName + "_";
        }
        stubName += methodName + PrinterUtils::KLEE_SYMBOLIC_SUFFIX;
        StringUtils::replaceColon(stubName); //TODO has problem for stubs
        return stubName;
    }

    std::string tryGetMethodNameFromStubSymbolic(const std::string &symbolicName) {
        std::string methodName = symbolicName.substr(
                0, symbolicName.size() - PrinterUtils::KLEE_SYMBOLIC_SUFFIX.size());
        return methodName;
    }
}
