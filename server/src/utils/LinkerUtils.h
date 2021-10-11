/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_LINKERUTILS_H
#define UNITTESTBOT_LINKERUTILS_H

#include "BuildResult.h"

#include "utils/path/FileSystemPath.h"
#include <string>

namespace LinkerUtils {
    fs::path applySuffix(const fs::path &output,
                         BuildResult::Type unitType,
                         const std::string &suffixForParentOfStubs);

};


#endif // UNITTESTBOT_LINKERUTILS_H
