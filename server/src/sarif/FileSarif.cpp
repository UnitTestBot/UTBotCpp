#include "FileSarif.h"
#include "gtest/gtest.h"
#include "utils/JsonUtils.h"
#include "Paths.h"
#include "loguru.h"
#include "utils/StringUtils.h"

namespace sarif {

    FileSarif::FileSarif(const tests::Tests &tests, bool writeFlag) : ProjectSarif(tests.sourceFileNameNoExt,
                                                                                   tests.relativeFileDir, "", writeFlag),
                                                                      sourcePath(tests.sourceFilePath) {}

    int FileSarif::generateSarifForFunction(tests::Tests::MethodDescription &methodDescription,
                                            const fs::path &projectPath) {
        int errorTestCases = 0;
        for (auto &testCase: methodDescription.testCases) {
            if (testCase.isError()) {
                errorTestCases++;
                if (!testCase.errorDescriptionInJson.has_value()) {
                    LOG_S(ERROR) << "Test Case is error but doesn't have json from klee";
                    continue;
                }
                fs::path jsonPath = testCase.errorDescriptionInJson.value();
                json testCaseJson = JsonUtils::getJsonFromFile(jsonPath);
                if (testCaseJson.at("locations").size() != 1) {
                    LOG_S(WARNING) << "Sarif locations have not 1, but " << testCaseJson.at("locations").size() << " objects";
                    continue;
                }
//                addCodeFlowWithoutExternal(testCaseJson, projectPath);
                if (testCaseJson.at("codeFlows").at(0).
                        at("threadFlows").at(0).at("locations").empty()) {
                    LOG_S(WARNING) << "Sarif stack trace is empty";
                    continue;
                }
                testCaseJson.at("locations").at(0) = testCaseJson.at("codeFlows").at(0).
                        at("threadFlows").at(0).at("locations").back().at("location");
                addResultToSarif(testCaseJson);
                testCase.sarif = this;
            }
        }
        LOG_S(INFO)
        << "Found " << errorTestCases << " error test cases for " << methodDescription.name << " in " << sourcePath;
        return errorTestCases;
    }




    void FileSarif::addCodeFlowWithoutExternal(json &result, const fs::path &projectRoot) {
//        deleteExternalFilesFromResult(result.at("codeFlows").at(0).at("threadFlows").at(0),
//                                      projectRoot, false);
    }

}