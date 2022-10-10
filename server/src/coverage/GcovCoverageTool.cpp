#include "GcovCoverageTool.h"

#include "Coverage.h"
#include "Paths.h"
#include "TimeExecStatistics.h"
#include "building/RunCommand.h"
#include "exceptions/CoverageGenerationException.h"
#include "utils/CollectionUtils.h"
#include "utils/ExecUtils.h"
#include "utils/FileSystemUtils.h"
#include "utils/JsonUtils.h"
#include "utils/LogUtils.h"
#include "utils/MakefileUtils.h"
#include "utils/StringUtils.h"
#include "utils/path/FileSystemPath.h"
#include "printers/DefaultMakefilePrinter.h"

#include "loguru.h"
#include "json.hpp"

#include <utility>

using Coverage::CoverageMap;
using Coverage::FileCoverage;

GcovCoverageTool::GcovCoverageTool(utbot::ProjectContext projectContext,
                                   ProgressWriter const *progressWriter)
    : CoverageTool(std::move(projectContext), progressWriter) {
}

std::vector<BuildRunCommand>
GcovCoverageTool::getBuildRunCommands(const std::vector<UnitTest> &testsToLaunch, bool withCoverage) {
    ExecUtils::throwIfCancelled();

    std::vector<BuildRunCommand> result;
    ExecUtils::doWorkWithProgress(
        testsToLaunch, progressWriter, "Collecting build and run commands",
        [&](UnitTest const &testToLaunch) {
            auto makefile = Paths::getMakefilePathFromSourceFilePath(
                projectContext,
                Paths::testPathToSourcePath(projectContext, testToLaunch.testFilePath));
            auto gtestFlags = getGTestFlags(testToLaunch);
            auto buildCommand = MakefileUtils::MakefileCommand(projectContext, makefile,
                                                               printer::DefaultMakefilePrinter::TARGET_BUILD,
                                                               gtestFlags);
            auto runCommand = MakefileUtils::MakefileCommand(projectContext, makefile,
                                                             printer::DefaultMakefilePrinter::TARGET_RUN, gtestFlags);
            result.push_back({testToLaunch, buildCommand, runCommand});
        });
    return result;
}

std::vector <std::string> GcovCoverageTool::getGcovArguments(bool jsonFormat) const {
    fs::path gcdaDirPath = Paths::getGcdaDirPath(projectContext);
    std::vector <fs::path> gcdaFiles = getGcdaFiles();
    if (gcdaFiles.empty()) {
        LOG_S(WARNING) << "There are no .gcda files in directory: " << gcdaDirPath;
        return {};
    }
    std::vector<std::string> gcovArgs = {"gcov"};
    if (jsonFormat) {
        gcovArgs.emplace_back("--json-format");
    }
    for (const auto& file : getGcdaFiles()) {
        gcovArgs.emplace_back(file.string());
    }
    return gcovArgs;
}

std::vector<ShellExecTask> GcovCoverageTool::getCoverageCommands(const std::vector<UnitTest> &testsToLaunch) {
    MEASURE_FUNCTION_EXECUTION_TIME
    fs::path gcovDir = Paths::getGccCoverageDir(projectContext);
    auto gcovArgs = getGcovArguments(true);
    if (gcovArgs.empty()) {
        return {};
    }
    fs::create_directories(gcovDir.string());
    return {
        ShellExecTask::getShellCommandTask("gcov", gcovArgs, gcovDir.string()),
        ShellExecTask::getShellCommandTask("gunzip", {"-f", "-r", gcovDir.string()})
    };
}

static void addLine(uint32_t lineNumber, bool covered, FileCoverage &fileCoverage) {
    assert(lineNumber > 0);
    if (covered) {
        fileCoverage.fullCoverageLines.insert({lineNumber - 1});
    } else {
        fileCoverage.noCoverageLines.insert({lineNumber - 1});
    }
}

static void setLineNumbers(const nlohmann::json &file, FileCoverage &fileCoverage) {
    for (const nlohmann::json &line : file.at("lines")) {
        uint32_t  lineNumber = line.at("line_number").get<int>();
        bool covered = line.at("count").get<int>() > 0;
        addLine(lineNumber, covered, fileCoverage);
    }
}

static void setFunctionBorders(const nlohmann::json &file, FileCoverage &fileCoverage) {
    for (const nlohmann::json &function : file.at("functions")) {
        uint32_t startLine = function.at("start_line");
        uint32_t endLine = function.at("end_line");
        bool covered = function.at("execution_count").get<int>() > 0;
        addLine(startLine, covered, fileCoverage);
        addLine(endLine, covered, fileCoverage);
    }
}

CoverageMap GcovCoverageTool::getCoverageInfo() const {
    ExecUtils::throwIfCancelled();

    CoverageMap coverageMap;

    auto covJsonDirPath = Paths::getGccCoverageDir(projectContext);
    if (!fs::exists(covJsonDirPath)) {
        std::string message = "Couldn't find coverage directory at " + covJsonDirPath.string();
        LOG_S(ERROR) << message;
        throw CoverageGenerationException(message);
    }
    LOG_S(INFO) << "Reading coverage files";

    ExecUtils::doWorkWithProgress(
        FileSystemUtils::DirectoryIterator(covJsonDirPath), progressWriter,
        "Reading coverage files", [&coverageMap](auto const &entry) {
            try {
                auto jsonPath = entry.path();
                auto coverageJson = JsonUtils::getJsonFromFile(jsonPath);
                for (const nlohmann::json &jsonFile: coverageJson.at("files")) {
                    fs::path filePath(std::filesystem::path(jsonFile.at("file")));
                    if (Paths::isGtest(filePath)) {
                        continue;
                    }
                    setLineNumbers(jsonFile, coverageMap[filePath]);
                    setFunctionBorders(jsonFile, coverageMap[filePath]);
                }
            } catch (const std::exception &e) {
                return;
            }
        });
    return coverageMap;
}

nlohmann::json GcovCoverageTool::getTotals() const {
    MEASURE_FUNCTION_EXECUTION_TIME
    fs::path gcovDir = Paths::getGccCoverageDir(projectContext);
    std::vector <std::string> gcovArgs = getGcovArguments(false);
    auto task = ShellExecTask::getShellCommandTask("gcov", gcovArgs, gcovDir.string());
    auto [out, status, path] = task.run();
    if (status != 0) {
        LOG_S(ERROR) << "gcov exit code: " << status << ". See more info in logs: " << path.value();
        return {};
    }
    /**
     * Output has the following structure:
     * File 'file.c'
     * lines executed:100.00% of 2
     * creating file '...'
     *
     * The code below retrieves the percentages
     * for project source files.
     */
    uint32_t totalCovered = 0, totalLines = 0;
    std::vector <std::string> gcovOutput = StringUtils::split(out.c_str(), '\n');
    for (size_t i = 0; i < gcovOutput.size(); i++) {
        const auto& line = gcovOutput[i];
        if (StringUtils::startsWith(line, "File ")) {
            std::string filename = line.substr(6, (int)line.size() - 7);
            if (!Paths::isUTBotWrapper(filename)) {
                auto fileCoverage = StringUtils::splitByWhitespaces(gcovOutput[i + 1]);
                double percent = strtod(fileCoverage[1].c_str() + 9, nullptr);
                uint32_t linesNumber = stoi(fileCoverage.back());
                uint32_t linesCovered = percent / 100 * linesNumber;
                totalCovered += linesCovered;
                totalLines += linesNumber;
                i++;
            }
        }
    }
    return { {
        "lines", {
            { "count", totalLines },
            { "covered", totalCovered },
            { "percent", (double)totalCovered * 100 / totalLines }
            }
    } };
}

void GcovCoverageTool::cleanCoverage() const {
    fs::path gccCoverageDir = Paths::getGccCoverageDir(projectContext);
    auto gcdaFiles = getGcdaFiles();
    for (fs::path const &gcdaFile : gcdaFiles) {
        fs::remove(gcdaFile);
    }
    FileSystemUtils::removeAll(gccCoverageDir);
}
std::vector<fs::path> GcovCoverageTool::getGcdaFiles() const {
    std::vector<fs::path> result;
    fs::path gcdaDirPath = Paths::getGcdaDirPath(projectContext);
    if (!fs::exists(gcdaDirPath)) {
        return {};
    }
    for (const auto &entry : fs::recursive_directory_iterator(gcdaDirPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".gcda") {
            result.emplace_back(entry.path());
        }
    }
    return result;
}
