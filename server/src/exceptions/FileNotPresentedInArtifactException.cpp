/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "FileNotPresentedInArtifactException.h"

#include "utils/StringFormat.h"

FileNotPresentedInArtifactException::FileNotPresentedInArtifactException(const fs::path &filePath)
    : BaseException(createMessage(filePath)), filePath(filePath) {
}

std::string FileNotPresentedInArtifactException::createMessage(const fs::path &filePath) {
    return StringUtils::stringFormat("%s: %s", MESSAGE, filePath);
}
const fs::path &FileNotPresentedInArtifactException::getFilePath() const {
    return filePath;
}
