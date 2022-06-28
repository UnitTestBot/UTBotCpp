#include "FileSarif.h"
#include "gtest/gtest.h"
#include "utils/JsonUtils.h"
#include "Paths.h"
#include "loguru.h"
#include "utils/StringUtils.h"

namespace sarif {

    FileSarif::FileSarif(const tests::Tests &tests, bool writeFlag) : ProjectSarif(tests.sourceFileNameNoExt,
                                                                                   tests.relativeFileDir, writeFlag),
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
                addCodeFlowWithoutExternal(testCaseJson, projectPath);
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


    json &FileSarif::getUriFromLocation(json &location) {
        return location.at("physicalLocation").at("artifactLocation").at("uri");
    }

    void FileSarif::addCodeFlowWithoutExternal(json &result, const fs::path &projectRoot) {
        deleteExternalFilesFromResult(result.at("codeFlows").at(0).at("threadFlows").at(0),
                                      projectRoot, false);
    }

    void FileSarif::deleteExternalFilesFromResult(json &result, const fs::path &projectRoot, bool leaveFromLib) {
        for (int i = 0; i < result.at("locations").size(); ++i) {
            json &location = result.at("locations").at(i).at("location");
            std::string location_path = fs::path((std::string) getUriFromLocation(location));
            if (Paths::isSubPathOf(projectRoot, location_path)) {
                getUriFromLocation(location) = location_path.substr(projectRoot.string().size() + 1);
            }
        }
        auto it = std::remove_if(result.at("locations").begin(), result.at("locations").end(),
                                 [&](json &location) {
                                        return !fs::exists(projectRoot / (std::string)getUriFromLocation(location.at("location")));
                                 });
        result.at("locations").erase(it, result.at("locations").end());
    }

namespace {
    [[nodiscard]] int getErrorLineFromResult(const json& result) {
        return result.at("locations").at(0).at("physicalLocation")
            .at("region").at("startLine");
    }

    [[nodiscard]] std::string getErrorFileFromResult(const json& result) {
        return result.at("locations").at(0).at("physicalLocation")
            .at("artifactLocation").at("uri");
    }
}

    void FileSarif::addResultToSarif(const json &newResult) {
        json &all = sarifJson.at("runs").at(0).at("results");
        for (json &result : all) {
            if (getErrorFileFromResult(result) == getErrorFileFromResult(newResult) &&
                    getErrorLineFromResult(result) == getErrorLineFromResult(newResult)) {
                result.at("codeFlows").push_back(newResult.at("codeFlows").at(0));
                return;
            }
        }
        sarifJson.at("runs").at(0).at("results").push_back(newResult);
    }

}