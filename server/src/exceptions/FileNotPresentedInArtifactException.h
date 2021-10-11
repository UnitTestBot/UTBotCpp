/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_FILENOTPRESENTEDINARTIFACTEXCEPTION_H
#define UNITTESTBOT_FILENOTPRESENTEDINARTIFACTEXCEPTION_H

#include "BaseException.h"

#include "utils/path/FileSystemPath.h"

class FileNotPresentedInArtifactException : public BaseException {
    fs::path filePath;

public:
    static inline const std::string MESSAGE =
        "File is presented in compile/link commands for chosen target, but not included in "
        "target's artifact";

    explicit FileNotPresentedInArtifactException(const fs::path &filePath);

    const fs::path &getFilePath() const;

    static std::string createMessage(const fs::path &filePath);
};


#endif // UNITTESTBOT_FILENOTPRESENTEDINARTIFACTEXCEPTION_H
