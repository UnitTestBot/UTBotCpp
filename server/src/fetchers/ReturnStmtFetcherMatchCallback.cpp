/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "ReturnStmtFetcherMatchCallback.h"

#include "Fetcher.h"

using namespace clang;
using namespace Matchers;

ReturnStmtFetcherMatchCallback::ReturnStmtFetcherMatchCallback(Fetcher *parent)
    : parent(parent) {
}

void ReturnStmtFetcherMatchCallback::run(const MatchFinder::MatchResult &Result) {
    if (const auto *FS = Result.Nodes.getNodeAs<ReturnStmt>(RETURN)) {
        SourceManager &srcMng = Result.Context->getSourceManager();
        if (FS->children().empty()) {
            return;
        }
        auto child = FS->children().begin();
        while (canBeMissed(*child) && !child->children().empty()) {
            child = child->children().begin();
        }
        auto variableExpr = *child;
        if (!isa<DeclRefExpr>(variableExpr)) {
            return;
        }

        auto declrefExpr = cast<DeclRefExpr>(variableExpr);
        parent->returnVariables.insert(declrefExpr->getDecl()->getID());
    }
}
