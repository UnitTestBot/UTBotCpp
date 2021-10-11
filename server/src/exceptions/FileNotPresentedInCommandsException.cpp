/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "FileNotPresentedInCommandsException.h"

#include "utils/StringUtils.h"

FileNotPresentedInCommandsException::FileNotPresentedInCommandsException(const fs::path &filePath)
    : BaseException(createMessage(filePath)), filePath(filePath) {
}

std::string FileNotPresentedInCommandsException::createMessage(const fs::path &filePath) {
    return StringUtils::stringFormat("%s: %s", MESSAGE, filePath);
}
const fs::path &FileNotPresentedInCommandsException::getFilePath() const {
    return filePath;
}
