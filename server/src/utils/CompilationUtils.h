/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_COPMILATIONUTILS_H
#define UNITTESTBOT_COPMILATIONUTILS_H

#include <clang/Tooling/CompilationDatabase.h>
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

    static inline const std::string MOUNTED_CC_JSON_DIR_NAME = "utbot_build";

    static inline const std::string FULL_COMMAND_PATTERN = R"(cd "%s" && mkdir -p %s && %s)";

    std::string getBuildDirectoryName(CompilerName compilerName);

    std::shared_ptr<clang::tooling::CompilationDatabase>
    getCompilationDatabase(const fs::path &buildCommandsJsonPath);

    CompilerName getCompilerName(fs::path const &compilerPath);

    fs::path detectBuildCompilerPath(
        const std::shared_ptr<clang::tooling::CompilationDatabase> &compilationDatabase);

    fs::path substituteRemotePathToCompileCommandsJsonPath(const fs::path &projectPath,
                                                           const std::string &buildDirRelativePath);

    fs::path getClangCompileCommandsJsonPath(const fs::path &buildCommandsJsonPath);

    fs::path removeSharedLibraryVersion(const fs::path &sharedObjectFile);

    std::string getDefaultCompilerForSourceFile(const fs::path& sourceFilePath);

    fs::path getBundledCompilerPath(CompilerName compilerName);
}

#endif //UNITTESTBOT_COPMILATIONUTILS_H
