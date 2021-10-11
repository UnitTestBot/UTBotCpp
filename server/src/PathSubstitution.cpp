/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "PathSubstitution.h"

fs::path PathSubstitution::substituteLineFlag(const fs::path& path) const {
    if (path == lineFlagSubstitution.from)
        return lineFlagSubstitution.to;
    return path;
}
