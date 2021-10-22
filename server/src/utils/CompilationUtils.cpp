/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "CompilationUtils.h"

#include "JsonUtils.h"
#include "Paths.h"
#include "StringUtils.h"
#include "exceptions/CompilationDatabaseException.h"
#include "environment/EnvironmentPaths.h"

#include "loguru.hpp"

#include <fstream>

namespace CompilationUtils {
    using std::string;
    using std::shared_ptr;

    shared_ptr<clang::tooling::CompilationDatabase>
    getCompilationDatabase(const fs::path &buildCommandsJsonPath) {
        fs::path clangCompileCommandsJsonPath =
            getClangCompileCommandsJsonPath(buildCommandsJsonPath);
        string errorMessage;
        auto compilationDatabase = clang::tooling::CompilationDatabase::autoDetectFromDirectory(
            clangCompileCommandsJsonPath.string(), errorMessage);
        if (!errorMessage.empty()) {
            throw CompilationDatabaseException(errorMessage);
        }
        return compilationDatabase;
    }

    CompilerName getCompilerName(const fs::path &compilerPath) {
        auto compiler = compilerPath.filename().string();
        if (StringUtils::contains(compiler, CLANGXX_PATH)) {
            return CompilerName::CLANGXX;
        }
        if (StringUtils::contains(compiler, CLANG_PATH)) {
            return CompilerName::CLANG;
        }
        if (StringUtils::contains(compiler, GXX_PATH_PATTERN)) {
            return CompilerName::GXX;
        }
        if (StringUtils::contains(compiler, GCC_PATH_PATTERN)) {
            return CompilerName::GCC;
        }
        return CompilerName::UNKNOWN;
    }

    fs::path detectBuildCompilerPath(const shared_ptr<clang::tooling::CompilationDatabase> &compilationDatabase) {
        for (auto const &compileCommand : compilationDatabase->getAllCompileCommands()) {
            fs::path compilerPath = fs::weakly_canonical(compileCommand.CommandLine[0]);
            auto compilerName = getCompilerName(compilerPath);
            if (compilerName != CompilerName::UNKNOWN) {
                return compilerPath;
            }
        }
        throw CompilationDatabaseException("Cannot detect compiler");
    }

    std::string getBuildDirectoryName(CompilerName compilerName) {
        switch (compilerName) {
            case CompilerName::GCC:
                return "build_gcc";
            case CompilerName::GXX:
                return "build_gxx";
            case CompilerName::CLANG:
                return "build_clang";
            case CompilerName::CLANGXX:
                return "build_clangxx";
            default:
                throw CompilationDatabaseException("Build directory for your compiler could not be determined");
        }
    }

    void substituteRemotePathToCCJsonForFile(const fs::path &projectPath,
                                             const std::string &buildDirRelativePath,
                                             const std::string &jsonFileName,
                                             const fs::path &newJsonDir) {
        fs::path compileCommandsJsonPath = projectPath / buildDirRelativePath / jsonFileName;
        fs::create_directories(newJsonDir);
        if (!fs::exists(compileCommandsJsonPath)) {
            throw CompilationDatabaseException("Can't find " + compileCommandsJsonPath.string());
        }
        std::ifstream ifs(compileCommandsJsonPath);
        json json = json::parse(ifs);
        string projectPathStr = Paths::normalizedTrimmed(fs::absolute(projectPath)).string();
        Paths::removeBackTrailedSlash(projectPathStr);

        const std::string directoryFieldName = "directory";
        const std::string fileFieldName = "file";
        const std::string filesFieldName = "files";
        const std::string commandFieldName = "command";
        const std::string argumentsFieldName = "arguments";

        for (auto &cmd : json) {
            string directoryField = cmd[directoryFieldName];
            string userSystemProjectPath =
                Paths::subtractPath(directoryField, buildDirRelativePath);
            Paths::removeBackTrailedSlash(userSystemProjectPath);

            if (cmd.contains(commandFieldName)) {
                string commandField = cmd[commandFieldName];
                StringUtils::replaceAll(commandField, userSystemProjectPath, projectPathStr);
                cmd[commandFieldName] = commandField;
            }
            if (cmd.contains(argumentsFieldName)) {
                for (auto &arg : cmd[argumentsFieldName]) {
                    string argumentsField = arg;
                    StringUtils::replaceAll(argumentsField, userSystemProjectPath, projectPathStr);
                    arg = argumentsField;
                }
            }

            StringUtils::replaceAll(directoryField, userSystemProjectPath, projectPathStr);
            cmd[directoryFieldName] = directoryField;

            if (cmd.contains(fileFieldName)) {
                string fileField = cmd[fileFieldName];
                StringUtils::replaceAll(fileField, userSystemProjectPath, projectPathStr);
                cmd[fileFieldName] = fileField;
            } else {
                for (auto &currentFile : cmd[filesFieldName]) {
                    string currentFileField = currentFile;
                    StringUtils::replaceAll(currentFileField, userSystemProjectPath,
                                            projectPathStr);
                    currentFile = currentFileField;
                }
            }
        }
        fs::path newJsonPath = newJsonDir / jsonFileName;
        JsonUtils::writeJsonToFile(newJsonPath, json);
        LOG_S(DEBUG) << jsonFileName << " for mount is written to: " << newJsonDir;
    }

    fs::path
    substituteRemotePathToCompileCommandsJsonPath(
            const fs::path& projectPath,
            const std::string& buildDirRelativePath) {
        const std::string ccJsonFileName = "compile_commands.json";
        fs::path newCCJsonDir = projectPath / buildDirRelativePath / MOUNTED_CC_JSON_DIR_NAME;
        substituteRemotePathToCCJsonForFile(projectPath, buildDirRelativePath, ccJsonFileName, newCCJsonDir);
        substituteRemotePathToCCJsonForFile(projectPath, buildDirRelativePath, "link_commands.json", newCCJsonDir);
        return newCCJsonDir;
    }

    fs::path getClangCompileCommandsJsonPath(const fs::path &buildCommandsJsonPath) {
        return buildCommandsJsonPath / "utbot_clang" / "compile_commands.json";
    }

    std::string to_string(CompilerName compilerName) {
        switch (compilerName) {
            case CompilerName::GCC:
                return "GCC";
            case CompilerName::GXX:
                return "GXX";
            case CompilerName::CLANG:
                return "CLANG";
            case CompilerName::CLANGXX:
                return "CLANGXX";
            default:
                return "UNKNOWN";
        }
    }

    fs::path removeSharedLibraryVersion(const fs::path &sharedObjectFile) {
        std::string sPath = sharedObjectFile;
        int i = static_cast<int>(sPath.length()) - 1;
        while (i >= 0 && (sPath[i] == '.' || isdigit(sPath[i]))) {
            i--;
        }
        return sPath.substr(0, i + 1);
    }

    std::string getDefaultCompilerForSourceFile(const fs::path &sourceFilePath) {
        if (Paths::isCFile(sourceFilePath)) {
            return Paths::getUTBotClang();
        }
        return Paths::getUTBotClangPP();
    }

    fs::path getBundledCompilerPath(CompilerName compilerName) {
        switch (compilerName) {
            case CompilerName::GCC:
                return Paths::getGcc();
            case CompilerName::GXX:
                return Paths::getGpp();
            case CompilerName::CLANG:
                return Paths::getUTBotClang();
            case CompilerName::CLANGXX:
                return Paths::getUTBotClangPP();
            default:
                throw CompilationDatabaseException("Couldn't get bundled compiler path for current compiler name");
        }
    }
}