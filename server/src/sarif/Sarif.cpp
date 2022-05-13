#include "Sarif.h"
#include "gtest/gtest.h"
#include "utils/JsonUtils.h"
#include "Paths.h"
#include "loguru.h"

namespace sarif {
    Sarif::Sarif() {
        sarifJson["version"] = defaultVersion;
        sarifJson["$schema"] = defaultScheme;
        sarifJson["runs"] = json::array();
        sarifJson["runs"][0]["tool"]["driver"]["name"] = defaultToolName;
        sarifJson["runs"][0]["tool"]["driver"]["rules"] = json::array();
        sarifJson["runs"][0]["results"] = json::array();
    }

    void Sarif::writeSarifFile(const fs::path &jsonPath) {
        fs::path finalPath = jsonPath.string() + "/" + sarifName;
        LOG_S(INFO) << "Load Sarif to " << finalPath.string();
        JsonUtils::writeJsonToFile(finalPath, sarifJson);
    }

    std::size_t Sarif::loadRuns(const fs::path &runsFolder, const fs::path &projectRoot) {
        if (!fs::exists(runsFolder)) {
            LOG_S(WARNING) << runsFolder.string() << " path not valid";
            return -1;
        }
        CollectionUtils::FileSet allFiles = Paths::findFilesInFolder(runsFolder);
        std::size_t countErrorRuns = 0;
        for (const fs::path &file : allFiles) {
            if (file.string().rfind("__sarif") != std::string::npos) {
                countErrorRuns += 1;
                json sarifRunFromFile = JsonUtils::getJsonFromFile(file);
                deleteExternalFilesFromResult(sarifRunFromFile.at("codeFlows").at(0).at("threadFlows").at(0), projectRoot);
                sarifJson["runs"][0]["results"].push_back(sarifRunFromFile);
            }
        }
        return countErrorRuns;
    }

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
}
