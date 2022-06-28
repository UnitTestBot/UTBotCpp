#include "ProjectSarif.h"
#include "loguru.h"
#include "Paths.h"
#include "utils/FileSystemUtils.h"

namespace sarif {

    void ProjectSarif::init() {
        sarifJson["version"] = defaultVersion;
        sarifJson["$schema"] = defaultScheme;
        sarifJson["runs"] = json::array();
        sarifJson["runs"][0]["tool"]["driver"]["name"] = defaultToolName;
        sarifJson["runs"][0]["tool"]["driver"]["rules"] = json::array();
        sarifJson["runs"][0]["results"] = json::array();
    }

    ProjectSarif::ProjectSarif(const std::string &name, const fs::path &path, const fs::path &projectPath, bool writeFlag) :
        sarifName(name  + default_output_suffix + sarif_default_extension),
        outputPath(default_output_dir_name / path),
        projectPath(projectPath),
        writeFileFlag(writeFlag) {
        init();
    }

    ProjectSarif::ProjectSarif(const fs::path &projectPath, bool writeFlag) :
        ProjectSarif(sarif_default_name, "", projectPath, writeFlag) {}

    void ProjectSarif::writeSarifFileToTmp(const fs::path &tmpPath) {
        if (writeFileFlag) {
            fs::path sarifPath = tmpPath / outputPath / sarifName;
            LOG_S(INFO) << "Create FileSarif " << sarifPath;
            FileSystemUtils::writeToFile(sarifPath, sarifJson.dump(4));
        }
    }

    void ProjectSarif::writeCodeAnalysisFolder(const fs::path &tmpPath, const fs::path &projectPath) {
        CollectionUtils::FileSet allFiles = Paths::findFilesInFolder(tmpPath / default_output_dir_name);
        for (const auto &file : allFiles) {
            fs::path relativePath = fs::relative(file, tmpPath);
            json sarif = JsonUtils::getJsonFromFile(file);
            FileSystemUtils::writeToFile(projectPath / relativePath, sarif.dump(4));
        }
    }

    void ProjectSarif::joinSarifFiles(const fs::path &tmpPath) {
        CollectionUtils::FileSet allFiles = Paths::findFilesInFolder(tmpPath / outputPath);
        for (const auto &file : allFiles) {
            if (file.filename_without_extension() != sarif_default_name + default_output_suffix) {
                addResultsFromFile(file);
            }
        }
    }

    void ProjectSarif::addResultsFromFile(const fs::path &path) {
        if (path.extension() != sarif_default_extension) {
            LOG_S(ERROR) << "In " << outputPath << " found not sarif file: " << path;
            return;
        }
        json sarif = JsonUtils::getJsonFromFile(path);
        for (auto &result : sarif.at("runs").at(0).at("results")) {
            deleteExternalFilesFromResult(result);
            result.at("locations").at(0) = result.at("codeFlows").at(0).
                    at("threadFlows").at(0).at("locations").back().at("location");

            addResultToSarif(result);
        }
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

    void ProjectSarif::addResultToSarif(const json &newResult) {
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

    json &ProjectSarif::getUriFromLocation(json &location) {
        return location.at("physicalLocation").at("artifactLocation").at("uri");
    }

    void ProjectSarif::deleteExternalFilesFromResult(json &result) {
        for (json &codeFlow : result.at("codeFlows")) {
            for (json &threadFlow : codeFlow.at("threadFlows")) {
                for (int i = 0; i < threadFlow.at("locations").size(); ++i) {
                    json &location = threadFlow.at("locations").at(i).at("location");
                    std::string location_path = fs::path((std::string) getUriFromLocation(location));
                    if (Paths::isSubPathOf(projectPath, location_path)) {
                        getUriFromLocation(location) = location_path.substr(projectPath.string().size() + 1);
                    }
                }
                auto it = std::remove_if(threadFlow.at("locations").begin(), threadFlow.at("locations").end(),
                                         [&](json &location) {
                                             return !fs::exists(projectPath /
                                                                (std::string) getUriFromLocation(
                                                                        location.at("location")));
                                         });
                threadFlow.at("locations").erase(it, threadFlow.at("locations").end());
            }
        }
    }

    void ProjectSarif::addSarifResult(const ProjectSarif &sarif) {
        for (auto &result : sarif.sarifJson.at("runs").at(0).at("results")) {
            sarifJson.at("runs").at(0).at("results").push_back(result);
        }
    }

    void ProjectSarif::createTmpFolder(const fs::path &tmpDir) {
        fs::path dir = tmpDir / default_output_dir_name;
        if (fs::exists(dir)) {
            FileSystemUtils::removeAll(dir);
        }
        fs::create_directories(dir);
    }
}
