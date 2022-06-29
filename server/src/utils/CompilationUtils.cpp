#include "CompilationUtils.h"

#include "JsonUtils.h"
#include "Paths.h"
#include "StringUtils.h"
#include "exceptions/CompilationDatabaseException.h"
#include "tasks/ShellExecTask.h"
#include "environment/EnvironmentPaths.h"

#include "loguru.h"

#include <fstream>

namespace CompilationUtils {
    std::shared_ptr<CompilationDatabase>
    getCompilationDatabase(const fs::path &buildCommandsJsonPath) {
        fs::path clangCompileCommandsJsonPath =
            getClangCompileCommandsJsonPath(buildCommandsJsonPath);
        std::string errorMessage;
        auto compilationDatabase = CompilationDatabase::autoDetectFromDirectory(
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
        std::string projectPathStr = Paths::normalizedTrimmed(fs::absolute(projectPath)).string();
        Paths::removeBackTrailedSlash(projectPathStr);

        const std::string directoryFieldName = "directory";
        const std::string fileFieldName = "file";
        const std::string filesFieldName = "files";
        const std::string commandFieldName = "command";
        const std::string argumentsFieldName = "arguments";

        for (auto &cmd : json) {
            std::string directoryField = cmd[directoryFieldName];
            std::string userSystemProjectPath =
                Paths::subtractPath(directoryField, buildDirRelativePath);
            Paths::removeBackTrailedSlash(userSystemProjectPath);

            if (cmd.contains(commandFieldName)) {
                std::string commandField = cmd[commandFieldName];
                StringUtils::replaceAll(commandField, userSystemProjectPath, projectPathStr);
                cmd[commandFieldName] = commandField;
            }
            if (cmd.contains(argumentsFieldName)) {
                for (auto &arg : cmd[argumentsFieldName]) {
                    std::string argumentsField = arg;
                    StringUtils::replaceAll(argumentsField, userSystemProjectPath, projectPathStr);
                    arg = argumentsField;
                }
            }

            StringUtils::replaceAll(directoryField, userSystemProjectPath, projectPathStr);
            cmd[directoryFieldName] = directoryField;

            if (cmd.contains(fileFieldName)) {
                std::string fileField = cmd[fileFieldName];
                StringUtils::replaceAll(fileField, userSystemProjectPath, projectPathStr);
                cmd[fileFieldName] = fileField;
            } else {
                for (auto &currentFile : cmd[filesFieldName]) {
                    std::string currentFileField = currentFile;
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

    std::optional<fs::path> getResourceDirectory(const fs::path &buildCompilerPath) {
        auto compilerName = CompilationUtils::getCompilerName(buildCompilerPath);
        switch (compilerName) {
        case CompilerName::GCC:
        case CompilerName::GXX: {
            // /usr/bin/gcc -> /usr/lib/gcc/x86_64-linux-gnu/9/libgcc.a
            std::string command = StringUtils::stringFormat("%s -print-libgcc-file-name", buildCompilerPath);
            auto [output, status, outPath] = ShellExecTask::runPlainShellCommand(command);
            if (status == 0) {
                StringUtils::rtrim(output);
                // /usr/lib/gcc/x86_64-linux-gnu/9/libgcc.a -> /usr/lib/gcc/x86_64-linux-gnu/9
                fs::path resourceDirPath = fs::path(output).parent_path();
                if (fs::exists(resourceDirPath)) {
                    return resourceDirPath;
                } else {
                    LOG_S(ERROR) << "Resource directory doesn't exist: " << resourceDirPath;
                    return std::nullopt;
                }
            } else {
                LOG_S(ERROR) << "Command for detecting libgcc location failed: " << command;
                LOG_S(ERROR) << output;
                return std::nullopt;
            }
            break;
        }
        case CompilerName::CLANG:
        case CompilerName::CLANGXX: {
            // /utbot_distr/install/bin/clang -> /utbot_distr/install/lib/clang/10.0.1
            std::string command = StringUtils::stringFormat("%s -print-resource-dir", buildCompilerPath);
            auto [output, status, outPath] = ShellExecTask::runPlainShellCommand(command);
            if (status == 0) {
                StringUtils::rtrim(output);
                fs::path resourceDirPath = output;
                if (fs::exists(resourceDirPath)) {
                    return resourceDirPath;
                } else {
                    LOG_S(ERROR) << "Resource directory doesn't exist: " << resourceDirPath;
                    return std::nullopt;
                }
            } else {
                LOG_S(ERROR) << "Command for detecting resource dir failed: " << command;
                LOG_S(ERROR) << output;
                return std::nullopt;
            }
            break;
        }
        case CompilerName::UNKNOWN:
            return std::nullopt;
        }
    }
}
