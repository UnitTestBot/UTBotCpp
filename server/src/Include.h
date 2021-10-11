/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_INCLUDE_H
#define UNITTESTBOT_INCLUDE_H

#include "utils/path/FileSystemPath.h"

struct Include {
    bool is_angled;
    fs::path path;

    Include(bool isAngled, fs::path path);
};


#endif //UNITTESTBOT_INCLUDE_H
