/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_COMPILATIONDATABASE_H
#define UNITTESTBOT_COMPILATIONDATABASE_H

#include "utils/CollectionUtils.h"

#include <clang/Tooling/CompilationDatabase.h>

class CompilationDatabase {
public:
    explicit CompilationDatabase(
        std::unique_ptr<clang::tooling::CompilationDatabase> clangCompilationDatabase_);

    static std::unique_ptr<CompilationDatabase>
    autoDetectFromDirectory(fs::path const& SourceDir, std::string &ErrorMessage);

    const clang::tooling::CompilationDatabase &getClangCompilationDatabase() const;
    const CollectionUtils::FileSet &getAllFiles() const;
    const fs::path &getBuildCompilerPath() const;
    const std::optional<fs::path>& getResourceDir() const;
private:
    std::unique_ptr<clang::tooling::CompilationDatabase> clangCompilationDatabase;
    CollectionUtils::FileSet allFiles;
    fs::path buildCompilerPath;
    std::optional<fs::path> resourceDir;

    CollectionUtils::FileSet initAllFiles() const;
    fs::path initBuildCompilerPath();
};


#endif // UNITTESTBOT_COMPILATIONDATABASE_H
