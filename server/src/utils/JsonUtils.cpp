#include "JsonUtils.h"

#include "utils/FileSystemUtils.h"

#include "loguru.h"

#include "utils/path/FileSystemPath.h"
#include <fstream>

namespace JsonUtils {
    nlohmann::json getJsonFromFile(const fs::path &path) {
        std::ifstream stream(path.string());
        std::stringstream buffer;
        buffer << stream.rdbuf();
        try {
            nlohmann::json coverageJson = nlohmann::json::parse(buffer.str());
            return coverageJson;
        } catch (...) {
            LOG_S(ERROR) << buffer.str();
            throw;
        }
    }

    void writeJsonToFile(const fs::path &jsonPath, const nlohmann::json &json) {
        FileSystemUtils::writeToFile(jsonPath, json.dump(INDENT));
    }
}
