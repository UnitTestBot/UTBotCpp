#include "StubsStorage.h"

#include <utility>
#include "utils/StubsUtils.h"

void StubsStorage::registerStub(const std::string &methodName, std::shared_ptr<types::FunctionInfo> functionInfo,
                                std::optional<fs::path> stubsHeaderPath) {
    if (stubsHeaderPath.has_value()) {
        _stubsHeaders.insert(stubsHeaderPath.value());
    }
    _functions[StubsUtils::getStubSymbolicVarName(methodName + "_" + functionInfo->name)] = std::move(functionInfo);
}

std::optional<std::shared_ptr<types::FunctionInfo>>
StubsStorage::getFunctionInfoByKTestObjectName(const std::string &objectName) const {
    return CollectionUtils::getOptionalValue(_functions, objectName);
}

std::unordered_set<std::string> StubsStorage::getStubsHeaders() {
    return _stubsHeaders;
}
