#ifndef UNITTESTBOT_COPMILATIONUTILS_H
#define UNITTESTBOT_COPMILATIONUTILS_H

#include "building/CompilationDatabase.h"

#include "json.hpp"

#include "utils/path/FileSystemPath.h"
#include <memory>

namespace CompilationUtils {
    using json = nlohmann::json;

    enum class CompilerName {
        GCC,
        GXX,
        CLANG,
        CLANGXX,
        UNKNOWN
    };

    std::string to_string(CompilerName compilerName);

    inline static const std::string GCC_PATH_PATTERN = "gcc";
    inline static const std::string GXX_PATH_PATTERN = "g++";
    inline static const std::string CLANG_PATH = "clang";
    inline static const std::string CLANGXX_PATH = "clang++";

    static inline const std::string UTBOT_BUILD_DIR_NAME = "utbot_build";

    static inline const std::string FULL_COMMAND_PATTERN_WITH_CD = R"(cd "%s" && mkdir -p %s && %s)";
    static inline const std::string FULL_COMMAND_PATTERN = R"(mkdir -p %s && %s)";

    std::string getBuildDirectoryName(CompilerName compilerName);

    std::shared_ptr<CompilationDatabase>
    getCompilationDatabase(const fs::path &buildCommandsJsonPath);

    CompilerName getCompilerName(fs::path const &compilerPath);

    fs::path substituteRemotePathToCompileCommandsJsonPath(const fs::path &projectPath,
                                                           const std::string &buildDirRelativePath);

    fs::path getClangCompileCommandsJsonPath(const fs::path &buildCommandsJsonPath);

    fs::path removeSharedLibraryVersion(const fs::path &sharedObjectFile);

    std::string getDefaultCompilerForSourceFile(const fs::path& sourceFilePath);

    fs::path getBundledCompilerPath(CompilerName compilerName);

    std::optional<fs::path> getResourceDirectory(const fs::path& buildCompilerPath);

    std::string getIncludePath(const fs::path &includePath);
}

#endif //UNITTESTBOT_COPMILATIONUTILS_H
