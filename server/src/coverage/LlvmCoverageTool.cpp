#include "LlvmCoverageTool.h"

#include <utility>

#include "Coverage.h"
#include "Paths.h"
#include "TimeExecStatistics.h"
#include "environment/EnvironmentPaths.h"
#include "exceptions/CoverageGenerationException.h"
#include "utils/ArgumentsUtils.h"
#include "utils/CollectionUtils.h"
#include "utils/FileSystemUtils.h"
#include "utils/JsonUtils.h"
#include "utils/MakefileUtils.h"
#include "utils/StringUtils.h"
#include "utils/path/FileSystemPath.h"
#include "printers/DefaultMakefilePrinter.h"

#include "loguru.h"

using Coverage::CoverageMap;
using Coverage::FileCoverage;

LlvmCoverageTool::LlvmCoverageTool(utbot::ProjectContext projectContext,
                                   ProgressWriter const *progressWriter)
    : CoverageTool(std::move(projectContext), progressWriter) {
}

std::vector<BuildRunCommand>
LlvmCoverageTool::getBuildRunCommands(const std::vector<UnitTest> &testsToLaunch, bool withCoverage) {
    return CollectionUtils::transform(testsToLaunch, [&](UnitTest const &testToLaunch) {
        fs::path sourcePath =
            Paths::testPathToSourcePath(projectContext, testToLaunch.testFilePath);
        auto makefilePath = Paths::getMakefilePathFromSourceFilePath(projectContext, sourcePath);
        auto testName = testToLaunch.testname;
        auto gtestFlags = getGTestFlags(testToLaunch);
        std::vector<std::string> profileEnv;
        if (withCoverage) {
            auto profrawFilePath = Paths::getProfrawFilePath(projectContext, testName);
            profileEnv = {StringUtils::stringFormat("LLVM_PROFILE_FILE=%s", profrawFilePath)};
        }
        auto buildCommand = MakefileUtils::MakefileCommand(projectContext, makefilePath,
                                                           printer::DefaultMakefilePrinter::TARGET_BUILD,
                                                           gtestFlags, profileEnv);
        auto runCommand = MakefileUtils::MakefileCommand(projectContext, makefilePath,
                                                         printer::DefaultMakefilePrinter::TARGET_RUN,
                                                         gtestFlags, profileEnv);
        return BuildRunCommand{testToLaunch, buildCommand, runCommand};
    });
}

std::vector<ShellExecTask>
LlvmCoverageTool::getCoverageCommands(const std::vector<UnitTest> &testsToLaunch) {
    MEASURE_FUNCTION_EXECUTION_TIME
    std::vector<std::string> coverageCommands;
    auto profrawFilePaths =
        CollectionUtils::transform(testsToLaunch, [&](UnitTest const &testToLaunch) {
            return Paths::getProfrawFilePath(projectContext, testToLaunch.testname);
        });
    bool allEmpty = true;
    for (fs::path const &profrawFilePath : profrawFilePaths) {
        if (!fs::exists(profrawFilePath)) {
            LOG_S(WARNING) << "Profraw file is missing: " << profrawFilePath;
            return {};
        }
        allEmpty &= fs::is_empty(profrawFilePath);
    }
    if (allEmpty) {
        LOG_S(WARNING) << "All profraw files are empty: "
                       << StringUtils::joinWith(profrawFilePaths, " ");
        return {};
    }

    auto testFilenames = CollectionUtils::transformTo<CollectionUtils::FileSet>(
            testsToLaunch, [](UnitTest const &test) { return test.testFilePath; });
    auto objectFiles = CollectionUtils::transformTo<std::unordered_set<std::string>>(
            testFilenames, [this](fs::path const &testFilePath) {
                fs::path sourcePath = Paths::testPathToSourcePath(projectContext, testFilePath);
                fs::path makefile =
                        Paths::getMakefilePathFromSourceFilePath(projectContext, sourcePath);
                auto makefileCommand = MakefileUtils::MakefileCommand(projectContext, makefile, "bin");
                auto res = makefileCommand.run();
                if (res.status == 0) {
                    if (res.output.empty()) {
                        std::string message = "Coverage result empty. See logs for more information.";
                        LOG_S(ERROR) << message;
                        throw CoverageGenerationException(message);
                    }
                    return StringUtils::split(res.output, '\n').back();
                }
                std::string message =
                        "Coverage generation failed. See logs for more information.";
                LOG_S(ERROR) << message;
                throw CoverageGenerationException(message);
            });

    auto showObjectFiles = CollectionUtils::transformTo<std::unordered_set<std::string>>(
            testFilenames, [this](fs::path const &testFilePath) {
                fs::path sourcePath = Paths::testPathToSourcePath(projectContext, testFilePath);
                fs::path makefile =
                        Paths::getMakefilePathFromSourceFilePath(projectContext, sourcePath);
                auto makefileCommand = MakefileUtils::MakefileCommand(projectContext, makefile, "obj_path");
                auto res = makefileCommand.run();
                if (res.status == 0) {
                    if (res.output.empty()) {
                        std::string message = "Coverage result empty. See logs for more information.";
                        LOG_S(ERROR) << message;
                        throw CoverageGenerationException(message);
                    }
                    return StringUtils::split(res.output, '\n').back();
                }
                std::string message =
                        "Coverage generation failed. See logs for more information.";
                LOG_S(ERROR) << message;
                throw CoverageGenerationException(message);
            });

    fs::path mainProfdataPath = Paths::getMainProfdataPath(projectContext);
    std::vector<std::string> mergeArguments = {"merge"};
    for (const fs::path &profrawFile: profrawFilePaths) {
        mergeArguments.emplace_back(profrawFile.string());
    }
    mergeArguments.emplace_back("-o");
    mergeArguments.emplace_back(mainProfdataPath);
    auto mergeTask = ShellExecTask::getShellCommandTask(Paths::getLLVMprofdata(), mergeArguments);

    LOG_S(INFO) << "Merge coverage info command: " << mergeTask.toString();

    fs::path coverageJsonPath = Paths::getCoverageJsonPath(projectContext);
    fs::create_directories(coverageJsonPath.parent_path());
    std::vector<std::string> exportArguments = {"export"};
    std::vector<std::string> reportArguments = {"report"};
//    std::vector<std::string> gcovArgs = {"gcov", "-f", "-b", "-c"};

    // From documentation:
    //   llvm-cov export [options] -instr-profile PROFILE BIN [-object BIN,...] [[-object BIN]] [SOURCES]
    bool firstBIN = true; // the first BIN need to be mentioned without `-object`
    for (const std::string &objectFile: objectFiles) {
        if (firstBIN) {
            firstBIN = false;
        } else {
            exportArguments.emplace_back("-object");
            reportArguments.emplace_back("-object");
        }
        exportArguments.emplace_back(objectFile);
        reportArguments.emplace_back(objectFile);
    }

    for (const std::string &objectFile: showObjectFiles) {
        if (firstBIN) {
            firstBIN = false;
        } else {
            reportArguments.emplace_back("-object");
        }
        reportArguments.emplace_back(objectFile);
    }

    exportArguments.emplace_back("-instr-profile=" + mainProfdataPath.string());
    reportArguments.emplace_back("-instr-profile=" + mainProfdataPath.string());

    try {
        auto sourcePaths = CollectionUtils::transformTo<
                std::unordered_set<std::string>>(testFilenames, [this](fs::path const &testFilePath) {
            fs::path sourcePath = Paths::testPathToSourcePath(projectContext, testFilePath);
            if (!fs::exists(sourcePath)) {
                std::string message =
                        "Coverage generation: Source file `"
                        + sourcePath.string()
                        + "` does not exist. Wrongly restored from test file `"
                        + testFilePath.string()
                        + "`.";
                LOG_S(ERROR) << message;
                throw CoverageGenerationException(message);
            }
            return sourcePath.string();
        });
        for (const std::string &sourcePath: sourcePaths) {
            exportArguments.emplace_back(sourcePath);
            reportArguments.emplace_back(sourcePath);
        }
    }
    catch (const CoverageGenerationException &ce) {
        LOG_S(WARNING) << "Skip Coverage filtering for tested source files: "
                       << ce.what();
    }

    auto exportTask = ShellExecTask::getShellCommandTask(Paths::getLLVMcov(), exportArguments);
    exportTask.setLogFilePath(coverageJsonPath);
    exportTask.setRetainOutputFile(true);

    LOG_S(INFO) << "Export coverage command: " << exportTask.toString();


    reportArguments.emplace_back("-show-functions=true");
    auto exportFCTask = ShellExecTask::getShellCommandTask(Paths::getLLVMcov(), reportArguments);
    exportFCTask.setLogFilePath(Paths::getFunctionReportPath(projectContext));
    exportFCTask.setRetainOutputFile(true);

    LOG_S(INFO) << "Get coverage by functions command:" << exportFCTask.toString();

//    for (auto i: getGcdaFiles()) {
//        gcovArgs.emplace_back(i);
//    }

//    auto exportFCGcovTask = ShellExecTask::getShellCommandTask(Paths::getLLVMcov(), gcovArgs);
//    exportFCGcovTask.setLogFilePath(Paths::getFunctionReportPath(projectContext));
//    exportFCGcovTask.setRetainOutputFile(true);
//    LOG_S(INFO) << "Get coverage by functions gcov command:" << exportFCGcovTask.toString();


    return {mergeTask, exportTask, exportFCTask};
}

Coverage::CoverageMap LlvmCoverageTool::getCoverageInfo() const {
    CoverageMap coverageMap;
    fs::path covJsonPath = Paths::getCoverageJsonPath(projectContext);
    if (!fs::exists(covJsonPath)) {
        std::string message = StringUtils::stringFormat("Can't found coverage.json at %s", covJsonPath.string());
        LOG_S(ERROR) << message;
        throw CoverageGenerationException(message);
    }
    try {
        LOG_S(INFO) << "Reading coverage.json";
        nlohmann::json coverageJson = JsonUtils::getJsonFromFile(covJsonPath);

        // Parsing is based on LLVM coverage mapping format
        ExecUtils::doWorkWithProgress(
                coverageJson.at("data"), progressWriter, "Reading coverage.json",
                [&coverageMap](const nlohmann::json &data) {
                    for (const nlohmann::json &function: data.at("functions")) {
                        std::string filename = function.at("filenames").at(0);
                        // no need to show coverage for gtest library
                        if (Paths::isGtest(filename)) {
                            continue;
                        }
                        for (const nlohmann::json &region: function.at("regions")) {
                            // In an LLVM coverage mapping format a region is an array with line and
                            // character position
                            FileCoverage::SourcePosition startPosition{region.at(0).get<uint32_t>() - 1,
                                                                       region.at(1).get<uint32_t>() - 1};
                            FileCoverage::SourcePosition endPosition{region.at(2).get<uint32_t>() - 1,
                                                                     region.at(3).get<uint32_t>() - 1};
                            FileCoverage::SourceRange sourceRange{startPosition, endPosition};
                            // The 4th element in LLVM coverage mapping format of a region
                            if (region.at(4).get<int>() == 0) {
                                coverageMap[filename].uncoveredRanges.push_back(sourceRange);
                            } else if (region.at(4).get<int>() >= 1) {
                                coverageMap[filename].coveredRanges.push_back(sourceRange);
                            }
                        }
                    }
                });

        for (const auto &item: coverageMap) {
            countLineCoverage(coverageMap, item.first);
        }

        return coverageMap;
    } catch (const std::exception &e) {
        std::string message = "Can't parse coverage.json at " + covJsonPath.string();
        LOG_S(ERROR) << message;
        throw CoverageGenerationException(message);
    }
}

void LlvmCoverageTool::countLineCoverage(Coverage::CoverageMap &coverageMap,
                                         const std::string &filename) const {
    for (auto range: coverageMap[filename].uncoveredRanges) {
        coverageMap[filename].noCoverageLinesBorders.insert({range.start.line});
        coverageMap[filename].noCoverageLinesBorders.insert({range.end.line});
        for (uint32_t i = range.start.line; i <= range.end.line; i++) {
            coverageMap[filename].noCoverageLines.insert({i});
        }
    }
    for (auto range: coverageMap[filename].coveredRanges) {
        checkLineForPartial({range.start.line}, coverageMap[filename]);
        checkLineForPartial({range.end.line}, coverageMap[filename]);
        for (uint32_t i = range.start.line + 1; i < range.end.line; i++) {
            if (coverageMap[filename].noCoverageLines.count({i}) == 0) {
                coverageMap[filename].fullCoverageLines.insert({i});
            }
        }
    }
}

void LlvmCoverageTool::checkLineForPartial(Coverage::FileCoverage::SourceLine line,
                                           Coverage::FileCoverage &fileCoverage) const {
    if (fileCoverage.noCoverageLinesBorders.count(line) > 0) {
        fileCoverage.partialCoverageLines.insert(line);
        fileCoverage.noCoverageLines.erase(line);
    } else {
        fileCoverage.fullCoverageLines.insert(line);
    }
}

nlohmann::json LlvmCoverageTool::getTotals() const {
    try {
        fs::path covJsonPath = Paths::getCoverageJsonPath(projectContext);
        nlohmann::json coverageJson = JsonUtils::getJsonFromFile(covJsonPath);
        return coverageJson.at("data").back().at("totals");
    } catch (const std::exception &e) {
        return {{
                        "lines", {
                        {"count", 0},
                        {"covered", 0},
                        {"percent", (double) 0.0}
                }}};
    }
}

std::vector<fs::path> LlvmCoverageTool::getGcdaFiles() const {
    std::vector<fs::path> result;
    fs::path gcdaDirPath = Paths::getGcdaDirPath(projectContext);
    if (!fs::exists(gcdaDirPath)) {
        return {};
    }
    for (const auto &entry: fs::recursive_directory_iterator(gcdaDirPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".gcda") {
            result.emplace_back(entry.path());
        }
    }
    return result;
}

void LlvmCoverageTool::cleanCoverage() const {
    fs::path coverageDir = Paths::getClangCoverageDir(projectContext);
    FileSystemUtils::removeAll(coverageDir);


    fs::path gccCoverageDir = Paths::getGccCoverageDir(projectContext);
    auto gcdaFiles = getGcdaFiles();
    for (fs::path const &gcdaFile: gcdaFiles) {
        fs::remove(gcdaFile);
    }
    FileSystemUtils::removeAll(gccCoverageDir);
}
