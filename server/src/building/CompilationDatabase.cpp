/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */


#include "CompilationDatabase.h"

#include "Paths.h"
#include "exceptions/CompilationDatabaseException.h"
#include "utils/CompilationUtils.h"

CompilationDatabase::CompilationDatabase(
    std::unique_ptr<clang::tooling::CompilationDatabase> clangCompilationDatabase_)
    : clangCompilationDatabase(std::move(clangCompilationDatabase_)) {
    allFiles = initAllFiles();
    buildCompilerPath = initBuildCompilerPath();
    resourceDir = CompilationUtils::getResourceDirectory(buildCompilerPath);
}

CollectionUtils::FileSet CompilationDatabase::initAllFiles() const {
    auto files = clangCompilationDatabase->getAllFiles();
    return CollectionUtils::transformTo<CollectionUtils::FileSet>(
        files, [](fs::path const &path) { return fs::weakly_canonical(path); });
}

fs::path CompilationDatabase::initBuildCompilerPath() {
    for (auto const &compileCommand : clangCompilationDatabase->getAllCompileCommands()) {
        fs::path compilerPath = fs::weakly_canonical(compileCommand.CommandLine[0]);
        auto compilerName = CompilationUtils::getCompilerName(compilerPath);
        if (compilerName != CompilationUtils::CompilerName::UNKNOWN) {
            return compilerPath;
        }
    }
    throw CompilationDatabaseException("Cannot detect compiler");
}

const clang::tooling::CompilationDatabase &
CompilationDatabase::getClangCompilationDatabase() const {
    return *clangCompilationDatabase;
}

const CollectionUtils::FileSet &CompilationDatabase::getAllFiles() const {
    return allFiles;
}

const fs::path &CompilationDatabase::getBuildCompilerPath() const {
    return buildCompilerPath;
}

const std::optional<fs::path> &CompilationDatabase::getResourceDir() const {
    return resourceDir;
}
std::unique_ptr<CompilationDatabase>
CompilationDatabase::autoDetectFromDirectory(fs::path const& SourceDir, string &ErrorMessage) {
    auto clangCompilationDatabase = clang::tooling::CompilationDatabase::autoDetectFromDirectory(
        SourceDir.c_str(), ErrorMessage);
    if (clangCompilationDatabase == nullptr) {
        return nullptr;
    }
    return std::make_unique<CompilationDatabase>(std::move(clangCompilationDatabase));
}
