/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_FETCHERUTILS_H
#define UNITTESTBOT_FETCHERUTILS_H

#include "Paths.h"
#include "Tests.h"
#include "utils/CollectionUtils.h"
#include "TimeExecStatistics.h"
#include "loguru.h"

#include <clang/Tooling/Tooling.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

class ClangToolRunner {
    clang::IgnoringDiagConsumer ignoringDiagConsumer;
public:
    explicit ClangToolRunner(std::shared_ptr<clang::tooling::CompilationDatabase> compilationDatabase);

    template <typename ToolAction>
    void run(const fs::path &file,
             const ToolAction &toolAction,
             bool ignoreDiagnostics = false,
             std::optional<std::string> const &virtualFileContent = std::nullopt,
             bool onlySource = true) {
        MEASURE_FUNCTION_EXECUTION_TIME
        if (!Paths::isSourceFile(file) && (!Paths::isHeaderFile(file) || onlySource)) {
            return;
        }
        auto clangTool =
            std::make_unique<clang::tooling::ClangTool>(*compilationDatabase, file.string());
        if (ignoreDiagnostics) {
            clangTool->setDiagnosticConsumer(&ignoringDiagConsumer);
        }
        if (virtualFileContent.has_value()) {
            clangTool->mapVirtualFile(file.c_str(), virtualFileContent.value());
        }
        int status = clangTool->run(toolAction);
        if (!ignoreDiagnostics) {
            checkStatus(status);
        }
    }

    template <typename ToolAction>
    void run(tests::TestsMap *const tests,
             const ToolAction &toolAction,
             bool ignoreDiagnostics = false) {
        auto files = CollectionUtils::getKeys(*tests);
        for (fs::path const &file : files) {
            run(file, toolAction, ignoreDiagnostics);
        }
    }

    template <typename ToolAction>
    void runWithProgress(tests::TestsMap *const tests,
                         const ToolAction &toolAction,
                         const ProgressWriter *progressWriter,
                         std::string const &message,
                         bool ignoreDiagnostics = false) {
        MEASURE_FUNCTION_EXECUTION_TIME
        auto files = CollectionUtils::getKeys(*tests);
        ExecUtils::doWorkWithProgress(
            files, progressWriter, message,
            [&](fs::path const &file) { run(file, toolAction, ignoreDiagnostics); });
    }
private:
    std::shared_ptr<clang::tooling::CompilationDatabase> compilationDatabase;

    void checkStatus(int status) const;
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
