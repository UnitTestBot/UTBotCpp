/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_STUBS_H
#define UNITTESTBOT_STUBS_H

#include "utils/CollectionUtils.h"

#include <string>
#include <unordered_set>
#include <vector>
#include "utils/path/FileSystemPath.h"

using std::string;
using std::vector;

struct Stubs {
    fs::path filePath;
    string code;
    Stubs() = default;
    Stubs(fs::path filePath, string code) : filePath(std::move(filePath)), code(std::move(code)) {}
};

#endif // UNITTESTBOT_STUBS_H
