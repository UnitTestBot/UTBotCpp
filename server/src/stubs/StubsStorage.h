#ifndef UTBOTCPP_STUBSSTORAGE_H
#define UTBOTCPP_STUBSSTORAGE_H

#include <string>
#include <unordered_map>
#include <memory>

#include "types/Types.h"

class StubsStorage {
public:
    void registerStub(const std::string &methodName, std::shared_ptr<types::FunctionInfo>,
                      std::optional<fs::path> stubsHeaderPath = std::nullopt);

    std::optional<std::shared_ptr<types::FunctionInfo>>
    getFunctionInfoByKTestObjectName(const std::string &objectName) const;

    std::unordered_set<std::string> getStubsHeaders();

private:
    std::unordered_map<std::string, std::shared_ptr<types::FunctionInfo>> _functions;
    std::unordered_set<std::string> _stubsHeaders;
};


#endif //UTBOTCPP_STUBSSTORAGE_H
