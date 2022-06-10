#include "FileSarif.h"
#include "gtest/gtest.h"
#include "utils/JsonUtils.h"
#include "Paths.h"
#include "loguru.h"

namespace sarif {

    json getLocationFromResult(const json &result) {
        return result.at("locations").at(0);
    }

    std::string getFileFromResult(const json &result) {
        return getLocationFromResult(result).at("physicalLocation").at("artifactLocation").at("uri");
    }

    int getLineFromResult(const json &result) {
        return getLocationFromResult(result).at("physicalLocation").at("region").at("startLine");
    }

    FileSarif::FileSarif(const tests::Tests &tests, bool writeFlag) : ProjectSarif(tests.sourceFileNameNoExt,
                                                                                   tests.relativeFileDir, writeFlag),
                                                                      sourcePath(tests.sourceFilePath) {}

    int FileSarif::generateSarifForFunction(const tests::Tests::MethodDescription &methodDescription,
                                            const fs::path &projectPath) {
        int errorTestCases = 0;
        for (const auto &testCase: methodDescription.testCases) {
            if (testCase.isError()) {
                errorTestCases++;
                if (!testCase.errorDescriptionInJson.has_value()) {
                    LOG_S(ERROR) << "Test Case is error but doesn't have json from klee";
                    continue;
                }
                fs::path jsonPath = testCase.errorDescriptionInJson.value();
                json testCaseJson = JsonUtils::getJsonFromFile(jsonPath);
                string errorLocationStr = getUriFromLocation(testCaseJson.at("locations").at(0));
//                deleteExternalFilesFromResult(testCaseJson.at("codeFlows").at(0).at("threadFlows").at(0),
//                                              projectPath);
                addCodeFlowWithoutExternal(testCaseJson, projectPath);
                testCaseJson.at("locations").at(0) = testCaseJson.at("codeFlows").at(0).
                        at("threadFlows").at(0).at("locations").back().at("location");
                addResultToSarif(testCaseJson);
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
        json newResult = result;
        deleteExternalFilesFromResult(newResult.at("codeFlows").at(0).at("threadFlows").at(0),
                                      projectRoot);
        newResult.at("codeFlows").push_back(result.at("codeFlows").at(0));
        result = newResult;
    }

    void FileSarif::deleteExternalFilesFromResult(json &result, const fs::path &projectRoot) {
        for (int i = 0; i < result.at("locations").size(); ++i) {
            json &location = result.at("locations").at(i).at("location");
            string location_path = fs::path((string) getUriFromLocation(location));
            if (Paths::isSubPathOf(projectRoot, location_path)) {
                getUriFromLocation(location) = location_path.substr(projectRoot.string().size() + 1);
            }
        }
        auto it = std::remove_if(result.at("locations").begin(), result.at("locations").end(),
                                 [&](json &location) {
                                     return !fs::exists(
                                             projectRoot / (string) getUriFromLocation(location.at("location")));
                                 });
        result.at("locations").erase(it, result.at("locations").end());
    }

    void FileSarif::addResultToSarif(const json &result) {
        sarifJson.at("runs").at(0).at("results").push_back(result);
    }

}