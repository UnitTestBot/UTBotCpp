#ifndef UNITTESTBOT_ERRORINFO_H
#define UNITTESTBOT_ERRORINFO_H
#include <string>
#include <optional>
#include "path/FileSystemPath.h"

enum ErrorType {
    NO_ERROR,
    ASSERTION_FAILURE,
    EXCEPTION_THROWN
};

struct ErrorInfo {
    ErrorType errorType;
    std::string failureBody;
    fs::path fileWithFailure;
    uint64_t lineWithFailure;

    [[nodiscard]] ErrorInfo(ErrorType errorType_, std::string failureBody_, fs::path file, uint64_t line) : errorType(errorType_),
          failureBody(std::move(failureBody_)), fileWithFailure(std::move(file)), lineWithFailure(line) {}

    [[nodiscard]] ErrorInfo() : errorType(NO_ERROR), fileWithFailure(fs::path()), lineWithFailure(0) {}

    [[nodiscard]] ErrorInfo(ErrorType errorType_) : errorType(errorType_), fileWithFailure(), lineWithFailure(0) {}
};

#endif // UNITTESTBOT_ERRORINFO_H
