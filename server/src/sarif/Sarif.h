#ifndef UNITTESTBOT_SARIF_H
#define UNITTESTBOT_SARIF_H

#include <string>
#include "utils/JsonUtils.h"
#include "utils/path/FileSystemPath.h"
#include "loguru.h"

using json = nlohmann::json;
namespace sarif {
    struct Sarif {
    private:
        const std::string sarifName = "saifOutput.sarif";
        const std::string defaultVersion = "2.1.0";
        const std::string defaultToolName = "utbotSARIF";
        const std::string defaultScheme = "https://schemastore.azurewebsites.net/schemas/json/sarif-2.1.0-rtm.4.json";

        json sarifJson;

        void deleteExternalFilesFromResult(json &result, const fs::path &projectRoot);
        json &getUriFromLocation(json &location);

    public:
        const static inline std::string sarif_klee_prefix = "__sarif_";
        const static inline std::string sarif_klee_extension = ".json";
        const static inline std::string prefix_for_json_path = "// THIS LINE SHOULDN'T BE AT END, path of klee-sarif: ";

        Sarif();
        std::size_t loadRuns(const fs::path &jsonPath, const fs::path &projectRoot);
        void writeSarifFile(const fs::path &runsPath);
    };
}

#endif //UNITTESTBOT_SARIF_H
