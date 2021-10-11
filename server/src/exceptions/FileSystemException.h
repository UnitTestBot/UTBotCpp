/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_FILESYSTEMEXCEPTION_H
#define UNITTESTBOT_FILESYSTEMEXCEPTION_H

#include "BaseException.h"

#include "utils/path/FileSystemPath.h"

class FileSystemException : public BaseException {
public:
    FileSystemException(fs::filesystem_error const &error);
        
    FileSystemException(std::string message, fs::filesystem_error const &error);
};


#endif // UNITTESTBOT_FILESYSTEMEXCEPTION_H
