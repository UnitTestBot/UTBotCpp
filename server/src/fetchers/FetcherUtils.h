/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_FETCHERUTILS_H
#define UNITTESTBOT_FETCHERUTILS_H

#include "Paths.h"
#include "Tests.h"
#include "utils/CollectionUtils.h"
#include "TimeExecStatistics.h"

#include <clang/Tooling/Tooling.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

class ClangToolRunner {
    clang::IgnoringDiagConsumer ignoringDiagConsumer;
public:
    explicit ClangToolRunner(std::shared_ptr<clang::tooling::CompilationDatabase> compilationDatabase);

    void run(const fs::path &file,
             clang::tooling::ToolAction *toolAction,
             bool ignoreDiagnostics = false,
             std::optional<std::string> const &virtualFileContent = std::nullopt,
             bool onlySource = true);

    void run(const tests::TestsMap *tests,
             clang::tooling::ToolAction *toolAction,
             bool ignoreDiagnostics = false);

    void runWithProgress(const tests::TestsMap *tests,
                         clang::tooling::ToolAction *toolAction,
                         const ProgressWriter *progressWriter,
                         std::string const &message,
                         bool ignoreDiagnostics = false);
private:
    std::shared_ptr<clang::tooling::CompilationDatabase> compilationDatabase;
    std::optional<fs::path> resourceDir;

    void checkStatus(int status) const;

    void setResourceDirOption(clang::tooling::ClangTool *clangTool);
};

class ParamsHandler {
public:
    static types::Type getType(const clang::QualType &paramDef,
                               const clang::QualType &paramDecl,
                               const clang::SourceManager &sourceManager);

    static std::shared_ptr<types::FunctionInfo> getFunctionPointerDeclaration(
        const clang::FunctionType* fType,
        const string& fName,
        const clang::SourceManager& mng,
        bool isArray);
};

template <typename StatementType> bool canBeMissed(StatementType const& statement) {
    return llvm::isa<clang::ImplicitCastExpr>(statement) ||
           llvm::isa<clang::ParenExpr>(statement) || llvm::isa<clang::CStyleCastExpr>(statement);
}

#endif //UNITTESTBOT_FETCHERUTILS_H
