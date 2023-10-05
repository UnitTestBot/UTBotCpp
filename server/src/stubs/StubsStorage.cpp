#include "StubsStorage.h"

#include <utility>
#include "utils/StubsUtils.h"

void StubsStorage::registerFunctionPointerStub(const std::string &methodName,
                                               std::shared_ptr<types::FunctionInfo> functionInfo) {
    _functionPointers[StubsUtils::getStubSymbolicVarName(functionInfo->name, methodName)] = std::move(
            functionInfo);
}

std::optional<std::shared_ptr<types::FunctionInfo>>
StubsStorage::getFunctionPointerByKTestObjectName(const std::string &objectName) {
    return CollectionUtils::getOptionalValue(_functionPointers, objectName);
}
