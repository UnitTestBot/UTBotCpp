#ifndef CORE_JSONUTIL_H
#define CORE_JSONUTIL_H

#include "json.hpp"

#include "utils/path/FileSystemPath.h"

namespace JsonUtils {
    using json = nlohmann::json;

    static inline const int INDENT = 4;

    nlohmann::json getJsonFromFile(const fs::path &path);

    void writeJsonToFile(const fs::path &jsonPath, const nlohmann::json &json);
}

#endif //CORE_JSONUTIL_H
