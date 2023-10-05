#ifndef UTBOTCPP_STUBSSTORAGE_H
#define UTBOTCPP_STUBSSTORAGE_H

#include <string>
#include <unordered_map>
#include <memory>

#include "types/Types.h"

class StubsStorage {
public:
    void registerFunctionPointerStub(const std::string &methodName, std::shared_ptr<types::FunctionInfo>);

    std::optional<std::shared_ptr<types::FunctionInfo>>
    getFunctionPointerByKTestObjectName(const std::string &objectName);

private:
    std::unordered_map<std::string, std::shared_ptr<types::FunctionInfo>> _functionPointers{};
};


#endif //UTBOTCPP_STUBSSTORAGE_H
