/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "BordersFinder.h"

#include "Paths.h"
#include "clang-utils/ASTPrinter.h"
#include "clang-utils/Matchers.h"
#include "utils/CollectionUtils.h"
#include "utils/CompilationUtils.h"

#include "loguru.h"

using namespace clang;
using namespace llvm;
using namespace clang::ast_matchers;

BordersFinder::BordersFinder(const fs::path &filePath,
                             unsigned line,
                             const shared_ptr<CompilationDatabase> &compilationDatabase,
                             const fs::path &compileCommandsJsonPath)
        : line(line), classBorder(std::nullopt), clangToolRunner(compilationDatabase) {
    buildRootPath = Paths::subtractPath(compileCommandsJsonPath.string(), CompilationUtils::MOUNTED_CC_JSON_DIR_NAME);
    lineInfo.filePath = filePath;
}

void BordersFinder::run(const MatchFinder::MatchResult &Result) {
    LOG_SCOPE_FUNCTION(MAX);
    if (const auto *ST = Result.Nodes.getNodeAs<clang::CXXRecordDecl>(Matchers::STRUCT_OR_CLASS_JUST_DECL)) {
        SourceManager &sourceManager = Result.Context->getSourceManager();
        fs::path path = sourceManager.getFileEntryForID(sourceManager.getMainFileID())
                ->tryGetRealPathName()
                .str();
        auto borders = getBorders(sourceManager, ST->getSourceRange());
        if (!containsLine(borders) || (classBorder.has_value() && !(borders < classBorder.value()))) {
            return;
        }
        classBorder = borders;
        lineInfo.begin = borders.start.line;
        lineInfo.end = borders.end.line;
        lineInfo.scopeName = ST->getNameAsString();
        lineInfo.initialized = true;
        LOG_S(MAX) << "Class name: " << ST->getNameAsString();
        LOG_S(MAX) << "Class's borders: " << lineInfo.begin << ' ' << lineInfo.end;
    } else if (const FunctionDecl *FS; (FS = Result.Nodes.getNodeAs<FunctionDecl>(Matchers::FUNCTION_DEF))  ||
                                        (FS = Result.Nodes.getNodeAs<CXXConstructorDecl>(Matchers::CONSTRUCTOR_DEF))) {
        if (Result.Nodes.getNodeAs<CXXConstructorDecl>(Matchers::CONSTRUCTOR_DEF)) {
            lineInfo.isConstructor = true;
        }
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
                        if (line == borders.start.line) {
                            hasInnerChild = false;
                        } else {
                            lineInfo.wrapInBrackets = true;
                            lineInfo.insertAfter = false;
                        }
                    }
                    if (line == borders.start.line && isa<ReturnStmt>(currentStmt)) {
                        lineInfo.insertAfter = false;
                    }
                    break;
                }
            }
        }
        auto borders = getStmtBordersLines(sourceManager, currentStmt);
        lineInfo.begin = borders.start.line;
        lineInfo.end = borders.end.line;
        if (auto namedParent = dyn_cast<NamedDecl>(FS->getParent())) {
            lineInfo.scopeName = namedParent->getNameAsString();
        } else {
            lineInfo.scopeName = path.stem().string();
        }
        lineInfo.methodName = FS->getNameAsString();
        clang::QualType realReturnType = FS->getReturnType().getCanonicalType();
        if (const auto *CS = Result.Nodes.getNodeAs<CXXConstructorDecl>(Matchers::CONSTRUCTOR_DEF)) {
            realReturnType = CS->getThisObjectType();
        }
        lineInfo.functionReturnType = ParamsHandler::getType(realReturnType, realReturnType, sourceManager);
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
            int from = tempBorders.start.line;
            int to = tempBorders.end.line;
            if (to - from > 1) {
                break;
            }
            strRepresentation = ASTPrinter::getSourceText(parents[0].getSourceRange(), sourceManager);
            parents = Result.Context->getParents(parents[0]);
        }
        lineInfo.stmtString = strRepresentation;
        LOG_S(MAX) << "Method name: " << lineInfo.methodName << "\n"
                   << "Method's borders: " << borders.start.line << ' ' << borders.end.line;
        LOG_S(DEBUG) << "Statement string: " << lineInfo.stmtString;
    }
}

LineInfo BordersFinder::getLineInfo() {
    return lineInfo;
}

BordersFinder::Borders BordersFinder::getFunctionBordersLines(const SourceManager &srcMng, const FunctionDecl *FS) {
    auto currentStmt = FS->getBody();
    return getStmtBordersLines(srcMng, currentStmt);
}

BordersFinder::Borders BordersFinder::getStmtBordersLines(const SourceManager &srcMng, const Stmt *st) {
    return getStmtBordersLinesDynamic(srcMng, clang::ast_type_traits::DynTypedNode::create(*st));
}

BordersFinder::Borders BordersFinder::getStmtBordersLinesDynamic(const SourceManager &srcMng,
                                                                 const clang::ast_type_traits::DynTypedNode st) {
    auto sourceRange = st.getSourceRange();
    return getBorders(srcMng, sourceRange);
}

BordersFinder::Borders BordersFinder::getBorders(const SourceManager &srcMng, const SourceRange &sourceRange) {
    auto beginLine = srcMng.getExpansionLineNumber(sourceRange.getBegin());
    auto endLine = srcMng.getExpansionLineNumber(sourceRange.getEnd());
    auto beginColumn = srcMng.getExpansionColumnNumber(sourceRange.getBegin());
    auto endColumn = srcMng.getExpansionColumnNumber(sourceRange.getEnd());
    return {{beginLine, beginColumn},
            {endLine,   endColumn}};
}

bool BordersFinder::containsLine(BordersFinder::Borders b) const {
    return line >= b.start.line && line <= b.end.line;
}

void BordersFinder::findFunction() {
    MatchFinder finder;
    finder.addMatcher(Matchers::functionDefinitionMatcher, this);
    finder.addMatcher(Matchers::constructorDefinitionMatcher, this);
    finder.addMatcher(Matchers::memberConstructorDefinitionMatcher, this);
    auto factory = clang::tooling::newFrontendActionFactory(&finder);
    clangToolRunner.run(lineInfo.filePath, factory.get());
}

void BordersFinder::findClass() {
    MatchFinder finder;
    finder.addMatcher(Matchers::classJustDeclMatcher, this);
    finder.addMatcher(Matchers::structJustDeclMatcher, this);
    auto factory = clang::tooling::newFrontendActionFactory(&finder);
    clangToolRunner.run(lineInfo.filePath, factory.get(), false, std::nullopt, false);
}
