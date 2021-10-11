/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef CORE_JSONUTIL_H
#define CORE_JSONUTIL_H

#include <nlohmann/json.hpp>

#include "utils/path/FileSystemPath.h"

namespace JsonUtils {
    using json = nlohmann::json;

    static inline const int INDENT = 4;

    nlohmann::json getJsonFromFile(const fs::path &path);

    void writeJsonToFile(const fs::path &jsonPath, const nlohmann::json &json);
}

#endif //CORE_JSONUTIL_H
