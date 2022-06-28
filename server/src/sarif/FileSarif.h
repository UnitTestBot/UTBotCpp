#ifndef UNITTESTBOT_FILESARIF_H
#define UNITTESTBOT_FILESARIF_H

#include <string>
#include "utils/JsonUtils.h"
#include "utils/path/FileSystemPath.h"
#include "Tests.h"
#include "ProjectContext.h"
#include "ProjectSarif.h"
#include "loguru.h"

using json = nlohmann::json;
namespace sarif {
    struct FileSarif : ProjectSarif {
    private:
        const fs::path sourcePath;

        void addCodeFlowWithoutExternal(json &result, const fs::path &projectRoot);

    public:
        const static inline std::string sarif_klee_prefix = "__sarif_";
        const static inline std::string sarif_klee_extension = ".json";
        const static inline std::string prefix_for_json_path = "// THIS LINE SHOULDN'T BE AT END, path of klee-sarif: ";

        explicit FileSarif(const tests::Tests &tests, bool);
        int generateSarifForFunction(tests::Tests::MethodDescription &methodDescription, const fs::path &projectRoot);
    };
}

#endif //UNITTESTBOT_FILESARIF_H
