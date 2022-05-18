#ifndef UNITTESTBOT_SARIF_H
#define UNITTESTBOT_SARIF_H

#include <string>
#include "utils/JsonUtils.h"
#include "utils/path/FileSystemPath.h"
#include "Tests.h"
#include "ProjectContext.h"
#include "loguru.h"

using json = nlohmann::json;
namespace sarif {
    struct Sarif {
    private:
        const std::string sarifName;
        const std::string defaultVersion = "2.1.0";
        const std::string defaultToolName = "utbotSARIF";
        const std::string defaultScheme = "https://schemastore.azurewebsites.net/schemas/json/sarif-2.1.0-rtm.4.json";
        const fs::path sourcePath;
        const fs::path outputPath;

        json sarifJson;

        void deleteExternalFilesFromResult(json &result, const fs::path &projectRoot);
        json &getUriFromLocation(json &location);
        void addResultToSarif(const json &result);

    public:
        const static inline std::string sarif_klee_prefix = "__sarif_";
        const static inline std::string sarif_klee_extension = ".json";
        const static inline std::string sarif_default_extension = ".sarif";
        const static inline std::string prefix_for_json_path = "// THIS LINE SHOULDN'T BE AT END, path of klee-sarif: ";
        const static inline std::string default_sarif_name = "codeAnalysis";
        const static inline std::string default_output_dir_name = "codeAnalysis";
        const static inline std::string default_output_suffix = "_analysis";

        explicit Sarif(const tests::Tests &tests);
//        std::size_t loadSarifFromKlee(const fs::path &jsonPath, const utbot::ProjectContext &projectContext);
        int generateSarifForFunction(const tests::Tests::MethodDescription &methodDescription, const fs::path &projectRoot);
        void writeSarifFile(const fs::path &);
    };
}

#endif //UNITTESTBOT_SARIF_H
