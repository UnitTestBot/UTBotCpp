#ifndef UNITTESTBOT_LINEINFO_H
#define UNITTESTBOT_LINEINFO_H

#include <grpcpp/grpcpp.h>
#include <protobuf/testgen.grpc.pb.h>
#include <types/Types.h>

#include "utils/path/FileSystemPath.h"
#include <optional>
#include <string>

struct LineInfo {
    fs::path filePath;
    unsigned begin = 0;
    unsigned end = 0;
    std::string methodName;
    std::string scopeName;
    std::string stmtString; // only used for assert checking
    bool wrapInBrackets = false;
    bool insertAfter = true;
    bool initialized = false;
    [[maybe_unused]] types::Type functionReturnType;
    struct PredicateInfo {
        testsgen::ValidationType type = testsgen::ValidationType_MIN;
        std::string predicate = "";
        std::string returnValue = "";
    };
    std::optional<PredicateInfo> predicateInfo;
    bool forMethod = false;
    bool forClass = false;
    bool forAssert = false;
};

#endif // UNITTESTBOT_LINEINFO_H
