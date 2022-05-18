#include "Sarif.h"
#include "gtest/gtest.h"
#include "utils/JsonUtils.h"
#include "Paths.h"
#include "loguru.h"

namespace sarif {

    Sarif::Sarif(const tests::Tests &tests) : sarifName(tests.sourceFileNameNoExt + default_output_suffix + sarif_default_extension),
                                              sourcePath(tests.relativeFileDir),
                                              outputPath(default_output_dir_name / tests.relativeFileDir) {
        sarifJson["version"] = defaultVersion;
        sarifJson["$schema"] = defaultScheme;
        sarifJson["runs"] = json::array();
        sarifJson["runs"][0]["tool"]["driver"]["name"] = defaultToolName;
        sarifJson["runs"][0]["tool"]["driver"]["rules"] = json::array();
        sarifJson["runs"][0]["results"] = json::array();
    }

    void Sarif::writeSarifFile(const fs::path &projectPath) {
        fs::path sarifPath = projectPath / outputPath / sarifName;
        LOG_S(INFO) << "Create Sarif " << sarifPath;
        JsonUtils::writeJsonToFile(sarifPath, sarifJson);
    }

    int Sarif::generateSarifForFunction(const tests::Tests::MethodDescription &methodDescription,
                                        const fs::path &projectPath) {
        int errorTestCases = 0;
        for (const auto &testCase : methodDescription.testCases) {
            if (testCase.isError()) {
                errorTestCases++;
                fs::path jsonPath = testCase.errorDescriptionInJson.value();
                json testCaseJson = JsonUtils::getJsonFromFile(jsonPath);
                deleteExternalFilesFromResult(testCaseJson.at("codeFlows").at(0).at("threadFlows").at(0),
                                              projectPath);
                addResultToSarif(testCaseJson);
            }
        }
        LOG_S(INFO) << "Found " << errorTestCases << " error test cases for " << methodDescription.name << " in " << sourcePath;
        return errorTestCases;
    }


    /*
    std::size_t Sarif::loadSarifFromKlee(const fs::path &runsFolder, const utbot::ProjectContext &projectContext) {
        if (!fs::exists(runsFolder)) {
            LOG_S(WARNING) << runsFolder.string() << " path not valid";
            return 0;
        }
        CollectionUtils::FileSet allFiles = Paths::findFilesInFolder(runsFolder);
        std::size_t countErrorRuns = 0;
        for (const fs::path &file : allFiles) {
            if (file.string().rfind(sarif_klee_prefix) != std::string::npos) {
                countErrorRuns++;
                json sarifRunFromFile = JsonUtils::getJsonFromFile(file);
                string filename = file.filename_without_extension();
                fs::path relativeLocation =
                        fs::path(Sarif::default_output_dir_name) / Paths::getRelativeDirPath(projectContext, file);
//                Sarif sarif("", "");
                deleteExternalFilesFromResult(sarifRunFromFile.at("codeFlows").at(0).at("threadFlows").at(0),
                                              projectContext.projectPath);

//                sarifJson["runs"][0]["results"].push_back(sarifRunFromFile);
            }
        }
        return countErrorRuns;
    }
     */

    json &Sarif::getUriFromLocation(json &location) {
        return location.at("location").at("physicalLocation").at("artifactLocation").at("uri");
    }

    void Sarif::deleteExternalFilesFromResult(json &result, const fs::path &projectRoot) {
        for (int i = 0; i < result.at("locations").size(); ++i) {
            json &location = result.at("locations").at(i);
            string location_path = fs::path((string)getUriFromLocation(location));
            if (Paths::isSubPathOf(projectRoot, location_path)) {
                getUriFromLocation(location) = location_path.substr(projectRoot.string().size() + 1);
            }
        }
        auto it = std::remove_if(result.at("locations").begin(), result.at("locations").end(), [&](json &location) {
            return !fs::exists(projectRoot / (string)getUriFromLocation(location));
        });
        result.at("locations").erase(it, result.at("locations").end());
    }

    void Sarif::addResultToSarif(const json &result) {
        sarifJson.at("runs").at(0).at("results").push_back(result);
    }
}
