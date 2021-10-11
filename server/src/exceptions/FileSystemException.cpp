/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "FileSystemException.h"

#include "utils/StringUtils.h"

FileSystemException::FileSystemException(const fs::filesystem_error &error)
    : BaseException(error.what()) {
}
FileSystemException::FileSystemException(std::string message, const fs::filesystem_error &error)
    : BaseException(StringUtils::stringFormat("%s, %s", message, error.what())) {
}
