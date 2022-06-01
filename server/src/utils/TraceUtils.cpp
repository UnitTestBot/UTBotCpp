#include <string>
#include <fstream>
#include "ErrorInfo.h"
#include "utils/path/FileSystemPath.h"
#include "../Paths.h"

static std::string getLineByNumberInFileWithStackTrace(const fs::path& errorFilePath, int lineNumber) {
    std::ifstream input;
    std::string line;
    input.open(errorFilePath.string(), std::ios::in);
    for (int i = 0; i < lineNumber; i++) {
        if (input) {
            getline(input, line);
        }
    }
    return line;
}

[[nodiscard]]std::string getTypeOfError(const fs::path& errorFilePath) {
    return getLineByNumberInFileWithStackTrace(errorFilePath, 0);
}

[[nodiscard]]fs::path getFileErrorFound(const fs::path& errorFilePath) {
    std::string lineWithErrorFilePath = getLineByNumberInFileWithStackTrace(errorFilePath, 1);
    fs::path fileWithError(lineWithErrorFilePath.substr(6));
    return fileWithError;
}

[[nodiscard]]uint64_t getLineErrorFound(const fs::path& errorFilePath) {
    std::string stringWithLine = getLineByNumberInFileWithStackTrace(errorFilePath, 3);
    return std::stoi(stringWithLine.substr(6));
}

bool isPointerOutOfBound(const std::string& typeofError) {
    return typeofError == "Error: memory error: out of bound pointer";
}

bool errorInExceptionHeader(const std::string& fileWhereErrorFound) {
    return fileWhereErrorFound.find("exception.h") != std::string::npos;
}

[[nodiscard]]ErrorInfo getErrorInfo(const fs::path &path) {
    if (Paths::errorFileExists(path, "uncaught_exception") ||
        Paths::errorFileExists(path, "unexpected_exception")) {
        return {ErrorType::EXCEPTION_THROWN};
    }
    if (Paths::errorFileExists(path, "ptr")) {
        fs::path errorFilePath = Paths::replaceExtension(path, ".ptr.err");
        std::string typeOfError = getTypeOfError(errorFilePath);
        fs::path fileWhereErrorFound = getFileErrorFound(errorFilePath);
        if (isPointerOutOfBound(typeOfError) && errorInExceptionHeader(fileWhereErrorFound)) {
            // TODO: add other check that exception wah thrown: now klee generates "pointer out
            //  of bound in exception.h" instead of "exception was thrown"
            return {ErrorType::EXCEPTION_THROWN};
        }
    }
    if (Paths::errorFileExists(path, "assert")) {
        ErrorInfo errorInfo;
        errorInfo.errorType = ErrorType::ASSERTION_FAILURE;
        fs::path errorFilePath = Paths::replaceExtension(path, ".assert.err");
        errorInfo.failureBody = getTypeOfError(errorFilePath);
        errorInfo.fileWithFailure = getFileErrorFound(errorFilePath);
        errorInfo.lineWithFailure = getLineErrorFound(errorFilePath);
        return errorInfo;
    }
    return {ErrorType::NO_ERROR};
}

//[[nodiscard]]ErrorInfo hasFailedAssert(const fs::path& path) {
//    ErrorInfo errorInfo;
//    if (Paths::errorFileExists(path, "assert")) {
//        errorInfo.errorType = ErrorType::ASSERTION_FAILURE;
//        fs::path errorFilePath = Paths::replaceExtension(path, ".assert.err");
//        errorInfo.assertBody = getTypeOfError(errorFilePath);
//        errorInfo.fileWithFailure = getFileErrorFound(errorFilePath);
//        errorInfo.lineWithFailure = getLineErrorFound(errorFilePath);
//    }
//    return errorInfo;
//}
