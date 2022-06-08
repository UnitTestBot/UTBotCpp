#ifndef UNITTESTBOT_EXECUTIONPROCESSEXCEPTION_H
#define UNITTESTBOT_EXECUTIONPROCESSEXCEPTION_H

#include <exception>
#include <string>
#include <utility>
#include "utils/path/FileSystemPath.h"

struct ExecutionProcessException : public std::exception {
    explicit ExecutionProcessException(std::string cmd, fs::path logFilePath);

    [[nodiscard]] const char *what() const noexcept override;

    [[nodiscard]] std::string getLogFilePath() const noexcept;

private:
    std::string cmd;
    fs::path logFilePath;
};

#endif //UNITTESTBOT_EXECUTIONPROCESSEXCEPTION_H
