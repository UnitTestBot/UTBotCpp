#ifndef UNITTESTBOT_FILENOTPRESENTEDINCOMMANDSEXCEPTION_H
#define UNITTESTBOT_FILENOTPRESENTEDINCOMMANDSEXCEPTION_H

#include "BaseException.h"

#include "utils/path/FileSystemPath.h"

class FileNotPresentedInCommandsException : public BaseException {
    fs::path filePath;

public:
    static inline const std::string MESSAGE =
        "File is not presented in compile/link commands for chosen target";

    explicit FileNotPresentedInCommandsException(const fs::path &filePath);

    const fs::path &getFilePath() const;

    static std::string createMessage(const fs::path &filePath);
};


#endif // UNITTESTBOT_FILENOTPRESENTEDINCOMMANDSEXCEPTION_H
