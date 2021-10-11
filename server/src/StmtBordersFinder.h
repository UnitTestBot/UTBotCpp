/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_STMTBORDERSFINDER_H
#define UNITTESTBOT_STMTBORDERSFINDER_H

#include "fetchers/FetcherUtils.h"
#include "LineInfo.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/Rewrite/Frontend/Rewriters.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>

#include "utils/path/FileSystemPath.h"
#include <string>
#include <utility>

using std::string;
using std::unique_ptr;
using std::shared_ptr;

class StmtBordersFinder : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
    StmtBordersFinder(const fs::path &filePath,
                      unsigned line,
                      const shared_ptr<clang::tooling::CompilationDatabase> &compilationDatabase,
                      const fs::path &compileCommandsJsonPath);

    void run(const clang::ast_matchers::MatchFinder::MatchResult &Result) override;

    void launch();

    LineInfo getLineInfo();

private:
    unsigned line;
    LineInfo lineInfo{};
    fs::path buildRootPath;
    ClangToolRunner clangToolRunner;

    // TODO: use rewriter for insertion
    clang::Rewriter rewrt;

    static std::pair<unsigned, unsigned> getStmtBordersLines(const clang::SourceManager &srcMng, const clang::Stmt *st);

    static std::pair<unsigned, unsigned> getStmtBordersLinesDynamic(const clang::SourceManager &srcMng, clang::ast_type_traits::DynTypedNode st);

    [[nodiscard]] bool containsLine(std::pair<unsigned, unsigned> b) const;

    static std::pair<unsigned int, unsigned int>
    getStmtBordersLines(const clang::SourceManager &srcMng, const clang::SourceRange &sourceRange);

    std::pair<unsigned int, unsigned int>
    getFunctionBordersLines(const clang::SourceManager &srcMng, const clang::FunctionDecl *FS);
};


#endif //UNITTESTBOT_STMTBORDERSFINDER_H
