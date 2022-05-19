#ifndef UNITTESTBOT_PROJECTSARIF_H
#define UNITTESTBOT_PROJECTSARIF_H

#include <string>
#include "utils/JsonUtils.h"

using json = nlohmann::json;

namespace sarif {
    struct ProjectSarif {
    private:
        void init();
        void addResultsFromFile(const fs::path &);

    protected:
        const std::string defaultVersion = "2.1.0";
        const std::string defaultToolName = "utbotSARIF";
        const std::string defaultScheme = "https://schemastore.azurewebsites.net/schemas/json/sarif-2.1.0-rtm.4.json";
        const static inline std::string default_output_dir_name = "codeAnalysis";
        const static inline std::string default_output_suffix = "_analysis";
        const static inline std::string sarif_default_extension = ".sarif";
        const static inline std::string sarif_default_name = "project_code";

        json sarifJson;
        const std::string sarifName = "project_code";
        const fs::path outputPath = default_output_dir_name;

    public:
        ProjectSarif();
        ProjectSarif(const std::string &, const fs::path &);

        void writeSarifFile(const fs::path &);

        void joinSarifFiles(const fs::path &);

    };
}

#endif //UNITTESTBOT_PROJECTSARIF_H
