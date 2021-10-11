/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "StmtBordersFinder.h"

#include "Paths.h"
#include "clang-utils/ASTPrinter.h"
#include "clang-utils/Matchers.h"
#include "utils/CollectionUtils.h"
#include "utils/CompilationUtils.h"

#include "loguru.h"

using namespace clang;
using namespace llvm;
using namespace clang::ast_matchers;
using namespace clang::tooling;

StmtBordersFinder::StmtBordersFinder(const fs::path &filePath,
                                     unsigned line,
                                     const shared_ptr<CompilationDatabase> &compilationDatabase,
                                     const fs::path &compileCommandsJsonPath)
: line(line), clangToolRunner(compilationDatabase) {
    buildRootPath = Paths::subtractPath(compileCommandsJsonPath.string(), CompilationUtils::MOUNTED_CC_JSON_DIR_NAME);
    lineInfo.filePath = filePath;
}

void StmtBordersFinder::run(const MatchFinder::MatchResult &Result) {
    LOG_SCOPE_FUNCTION(MAX);
    if (const auto *FS = Result.Nodes.getNodeAs<FunctionDecl>(Matchers::FUNCTION_DEF)) {
        SourceManager &sourceManager = Result.Context->getSourceManager();

        fs::path path = sourceManager.getFileEntryForID(sourceManager.getMainFileID())
                            ->tryGetRealPathName()
                            .str();
        Stmt *currentStmt = FS->getBody();
        if ((currentStmt == nullptr) || !containsLine(getFunctionBordersLines(sourceManager, FS))) {
            return;
        }
        bool hasInnerChild = true;
        while (hasInnerChild) {
            hasInnerChild = false;
            for (auto child : currentStmt->children()) {
                if (child == nullptr) {
                    continue;
                }
                auto borders = getStmtBordersLines(sourceManager, child);
                if (containsLine(borders)) {
                    currentStmt = child;
                    hasInnerChild = true;
                    if (isa<IfStmt>(currentStmt) || isa<ForStmt>(currentStmt) ||
                        isa<WhileStmt>(currentStmt)) {
                        if (line == borders.first) {
                            hasInnerChild = false;
                        } else {
                            lineInfo.wrapInBrackets = true;
                            lineInfo.insertAfter = false;
                        }
                    }
                    if (line == borders.first && isa<ReturnStmt>(currentStmt)) {
                        lineInfo.insertAfter = false;
                    }
                    break;
                }
            }
        }
        auto *nodeParent = (CXXRecordDecl *)FS->getParent();
        auto borders = getStmtBordersLines(sourceManager, currentStmt);
        lineInfo.begin = borders.first;
        lineInfo.end = borders.second;
        lineInfo.scopeName = nodeParent != nullptr ? nodeParent->getNameAsString() : path.stem().string();
        lineInfo.methodName = FS->getNameAsString();
        const clang::QualType realReturnType = FS->getReturnType().getCanonicalType();
        lineInfo.functionReturnType = ParamsHandler::getType(realReturnType, realReturnType);
        lineInfo.initialized = true;

        string strRepresentation = ASTPrinter::getSourceText(currentStmt->getSourceRange(), sourceManager);
        auto parents = Result.Context->getParents(*currentStmt);
        const int MAX_ITERATIONS = 50;
        // if more than MAX_ITERATIONS happen, something is wrong
        for (int it = 0; it < MAX_ITERATIONS; ++it) {
            if (parents.empty()) {
                break;
            }
            auto tempBorders = getStmtBordersLinesDynamic(sourceManager, parents[0]);
            int from = tempBorders.first;
            int to = tempBorders.second;
            if (to - from > 1) {
                break;
            }
            strRepresentation = ASTPrinter::getSourceText(parents[0].getSourceRange(), sourceManager);
            parents = Result.Context->getParents(parents[0]);
        }
        lineInfo.stmtString = strRepresentation;
        LOG_S(MAX) << "Method name: " << lineInfo.methodName << "\n"
                   << "Method's borders: " << borders.first << ' ' << borders.second;
        LOG_S(DEBUG) << "Statement string: " << lineInfo.stmtString;
    }
}

LineInfo StmtBordersFinder::getLineInfo() {
    return lineInfo;
}

std::pair<unsigned, unsigned> StmtBordersFinder::getFunctionBordersLines(const SourceManager &srcMng, const FunctionDecl *FS) {
    auto currentStmt = FS->getBody();
    auto bodyBorders = getStmtBordersLines(srcMng, currentStmt);
    auto declBorders = getStmtBordersLines(srcMng, FS->getSourceRange());
    return {declBorders.first, bodyBorders.second};
}

std::pair<unsigned, unsigned> StmtBordersFinder::getStmtBordersLines(const SourceManager &srcMng, const Stmt *st) {
    return getStmtBordersLinesDynamic(srcMng, clang::ast_type_traits::DynTypedNode::create(*st));
}

std::pair<unsigned, unsigned> StmtBordersFinder::getStmtBordersLinesDynamic(const SourceManager &srcMng,
                                                                            const clang::ast_type_traits::DynTypedNode st) {
    auto sourceRange = st.getSourceRange();
    return getStmtBordersLines(srcMng, sourceRange);
}

std::pair<unsigned int, unsigned int>
StmtBordersFinder::getStmtBordersLines(const SourceManager &srcMng, const SourceRange &sourceRange) {
    auto beginLine = srcMng.getExpansionLineNumber(sourceRange.getBegin());
    auto endLine = srcMng.getExpansionLineNumber(sourceRange.getEnd());
    return { beginLine, endLine };
}

bool StmtBordersFinder::containsLine(std::pair<unsigned, unsigned> b) const {
    return line >= b.first && line <= b.second;
}

void StmtBordersFinder::launch() {
    MatchFinder finder;
    finder.addMatcher(Matchers::functionDefinitionMatcher, this);
    clangToolRunner.run(lineInfo.filePath, newFrontendActionFactory(&finder).get());
}
