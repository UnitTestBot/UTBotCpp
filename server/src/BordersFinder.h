#ifndef UNITTESTBOT_BORDERSFINDER_H
#define UNITTESTBOT_BORDERSFINDER_H

#include "fetchers/FetcherUtils.h"
#include "LineInfo.h"
#include "building/CompilationDatabase.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/Tooling/Tooling.h>

#include "utils/path/FileSystemPath.h"
#include <string>
#include <utility>

class BordersFinder : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
    BordersFinder(const fs::path &filePath,
                  unsigned line,
                  const std::shared_ptr<CompilationDatabase> &compilationDatabase,
                  const fs::path &compileCommandsJsonPath);

    void run(const clang::ast_matchers::MatchFinder::MatchResult &Result) override;

    void findFunction();

    void findClass();

    LineInfo getLineInfo();

private:
    unsigned line;
    LineInfo lineInfo{};
    struct Borders {
        struct Position {
            unsigned line;
            unsigned column;
        };
        Position start;
        Position end;

        friend bool operator<(const Borders& lhs, const Borders& rhs) {
            return (rhs.start.line < lhs.start.line ||
                   (rhs.start.line == lhs.start.line && rhs.start.column <= lhs.start.column)) &&
                   (rhs.end.line > lhs.start.line ||
                   (rhs.end.line == lhs.start.line && rhs.end.column >= lhs.start.column));
        }
    };
    std::optional<Borders> classBorder;
    ClangToolRunner clangToolRunner;

    // TODO: use rewriter for insertion

    Borders getStmtBordersLines(const clang::SourceManager &srcMng, const clang::Stmt *st);

    Borders getStmtBordersLinesDynamic(const clang::SourceManager &srcMng, clang::ast_type_traits::DynTypedNode st);

    [[nodiscard]] bool containsLine(Borders b) const;

    static Borders getBorders(const clang::SourceManager &srcMng, const clang::SourceRange &sourceRange);

    Borders getFunctionBordersLines(const clang::SourceManager &srcMng, const clang::FunctionDecl *FS);
};


#endif //UNITTESTBOT_BORDERSFINDER_H
