#include "Tests.h"
#include "ProjectContext.h"

namespace sarif {
    const std::string PREFIX_FOR_JSON_PATH = "// UTBOT_TEST_GENERATOR (function name,test index): ";

    const std::string TEST_FILE_KEY = "TestFile";
    const std::string TEST_LINE_KEY = "TestLine";
    const std::string TEST_NAME_KEY = "TestName";
    const std::string ERROR_ID_KEY = "ErrorId";

    const std::string SARIF_FILE_NAME = "project_code_analysis.sarif";

    void sarifAddTestsToResults(const utbot::ProjectContext &projectContext,
                                const tests::Tests &tests,
                                nlohmann::json &results);
    std::string sarifPackResults(const nlohmann::json &results);
}
