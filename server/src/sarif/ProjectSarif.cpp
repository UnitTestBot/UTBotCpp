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

    ProjectSarif::ProjectSarif(const std::string &name, const fs::path &path, bool writeFlag) :
        sarifName(name  + default_output_suffix + sarif_default_extension),
        outputPath(default_output_dir_name / path),
        writeFileFlag(writeFlag) {
        init();
    }

    ProjectSarif::ProjectSarif(bool writeFlag) : ProjectSarif(sarif_default_name, "", writeFlag) {}

    void ProjectSarif::writeSarifFileToTmp(const fs::path &tmpPath) {
//        if (writeFileFlag) {
            fs::path sarifPath = tmpPath / outputPath / sarifName;
            LOG_S(INFO) << "Create FileSarif " << sarifPath;
            FileSystemUtils::writeToFile(sarifPath, sarifJson.dump(4));
//        }
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
            sarifJson.at("runs").at(0).at("results").push_back(result);
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
