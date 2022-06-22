#include "FileSarif.h"
#include "gtest/gtest.h"
#include "utils/JsonUtils.h"
#include "Paths.h"
#include "loguru.h"
#include "utils/StringUtils.h"

namespace sarif {

namespace {
    [[nodiscard]] fs::path getUtbotDir() {
        return fs::current_path().parent_path().parent_path();
    }

    [[nodiscard]] fs::path getKleeDir() {
        fs::path utbotDir = getUtbotDir();
        fs::path kleeDir = utbotDir / "submodules" / "klee";
        if (!fs::exists(kleeDir)) {
            LOG_S(ERROR) << "Klee directory is not " << kleeDir;
            return "/";
        }
        return kleeDir;
    }

    [[nodiscard]] fs::path getKleeRuntimeDir() {
        return getKleeDir() / "runtime";
    }

    [[nodiscard]] bool inKleeRuntimeDir(const fs::path &path) {
        if (fs::exists(path)) {
            return Paths::isSubPathOf(getKleeRuntimeDir(), path);
        }
        if (StringUtils::startsWith(path.c_str(), "runtime")) {
            fs::path absPath = getKleeRuntimeDir().parent_path() / path;
            return fs::exists(absPath);
        } else {
            return false;
        }
    }

    [[nodiscard]] bool inKleeTmpDir(const fs::path &path) {
        return fs::exists(Paths::tmpPath / path);
    }

    [[nodiscard]] std::optional<fs::path> isRelevant(const fs::path &path) {
        if (inKleeTmpDir(path)) {
            return std::nullopt;
        }
        if (inKleeRuntimeDir(path)) {
            fs::path relativePath = fs::relative(getKleeRuntimeDir().parent_path() / path,
                                                 getKleeRuntimeDir());
            bool b = (relativePath.string().find("lib") != std::string::npos);
            if (b) {
                return getKleeRuntimeDir().parent_path() / path;
            }
            return std::nullopt;
        }
        return path;
    }
}

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
        json resultWithAbs = result;
        json &locationsWithAbs = resultWithAbs.at("codeFlows").at(0).at("threadFlows").at(0);
        deleteExternalFilesFromResult(locationsWithAbs,
                                      projectRoot, true);
        result = resultWithAbs;
        json &locations = result.at("codeFlows").at(0).at("threadFlows").at(0);
        deleteExternalFilesFromResult(locations,
                                      projectRoot, false);
        if (locationsWithAbs.at("locations").size() != locations.at("locations").size()) {
            result.at("codeFlows").push_back(resultWithAbs.at("codeFlows").at(0));
        }
    }

    void FileSarif::deleteExternalFilesFromResult(json &result, const fs::path &projectRoot, bool leaveFromLib) {
        for (int i = 0; i < result.at("locations").size(); ++i) {
            json &location = result.at("locations").at(i).at("location");
            string location_path = fs::path((string) getUriFromLocation(location));
            if (Paths::isSubPathOf(projectRoot, location_path)) {
                getUriFromLocation(location) = location_path.substr(projectRoot.string().size() + 1);
            }
        }
        auto it = std::remove_if(result.at("locations").begin(), result.at("locations").end(),
                                 [&](json &location) {
                                        if (leaveFromLib) {
                                            auto opt = isRelevant(
                                                    (string) getUriFromLocation(location.at("location")));
                                            if (!opt.has_value()) {
                                                return true;
                                            }
                                            getUriFromLocation(location.at("location")) = opt.value().string();
                                            return false;
                                        } else {
                                            return StringUtils::startsWith(
                                                    (string) getUriFromLocation(location.at("location")), "/");
                                        }
                                 });
        result.at("locations").erase(it, result.at("locations").end());
    }

    void FileSarif::addResultToSarif(const json &result) {
        sarifJson.at("runs").at(0).at("results").push_back(result);
    }

}