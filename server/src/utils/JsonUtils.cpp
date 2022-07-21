#include "JsonUtils.h"

#include "utils/FileSystemUtils.h"

#include "loguru.h"

#include "utils/path/FileSystemPath.h"
#include <fstream>
#include <exception>

namespace JsonUtils {
    nlohmann::json getJsonFromFile(const fs::path &path) {
        std::ifstream stream(path.string());
        std::stringstream buffer;
        buffer << stream.rdbuf();
        try {
            nlohmann::json coverageJson = nlohmann::json::parse(buffer.str());
            return coverageJson;
        } catch (const std::exception &e) {
            LOG_S(ERROR) << e.what() << ": " << buffer.str() << " in: " << path.string();
            throw e;
        } catch (...) {
            LOG_S(ERROR) << buffer.str();
            throw;
        }
    }

    void writeJsonToFile(const fs::path &jsonPath, const nlohmann::json &json) {
        FileSystemUtils::writeToFile(jsonPath, json.dump(INDENT));
    }
}
