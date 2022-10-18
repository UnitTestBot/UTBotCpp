#include "JsonUtils.h"
#include "StringUtils.h"
#include "utils/FileSystemUtils.h"
#include "utils/path/FileSystemPath.h"

#include "loguru.h"

#include <exception>
#include <fstream>

namespace JsonUtils {
    nlohmann::json getJsonFromFile(const fs::path &path) {
        std::ifstream stream(path.string());
        std::stringstream buffer;
        buffer << stream.rdbuf();
        std::string contentToParse = buffer.str();
        try {
            // skip warning header lines from input stream like
            //    warning: 1 functions have mismatched data
            //    {JSON}
            std::string::size_type pos = 0;
            while (pos < contentToParse.size()) {
                const char ch = contentToParse[pos];
                if (!std::isspace(ch)) {
                    if (ch == '{' || ch == '[') {
                        break;
                    }
                    pos = contentToParse.find('\n', pos);
                    if (pos == std::string::npos) {
                        const std::invalid_argument e("empty json file");
                        LOG_S(ERROR) << e.what();
                        throw e;
                    }
                }
                ++pos;
            }
            if (pos > 0) {
                std::string warningHeader = contentToParse.substr(0, pos);
                StringUtils::trim(warningHeader);
                if (!warningHeader.empty()) {
                    LOG_S(WARNING) << "JSON header: [" << warningHeader << "] in: " << path.string();
                }
            }
            nlohmann::json coverageJson = nlohmann::json::parse(contentToParse.substr(pos));
            return coverageJson;
        } catch (const std::exception &e) {
            LOG_S(ERROR) << e.what() << ": " << contentToParse << " in: " << path.string();
            throw e;
        } catch (...) {
            LOG_S(ERROR) << "Crash: " << contentToParse << " in: " << path.string();
            throw;
        }
    }

    void writeJsonToFile(const fs::path &jsonPath, const nlohmann::json &json) {
        FileSystemUtils::writeToFile(jsonPath, json.dump(INDENT));
    }
}
