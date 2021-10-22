/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "ArraySubscriptFetcherMatchCallback.h"

#include "Fetcher.h"
#include "clang-utils/Matchers.h"
#include "utils/CollectionUtils.h"

#include "loguru.h"

#include "utils/path/FileSystemPath.h"

using namespace clang;
using namespace Matchers;

ArraySubscriptFetcherMatchCallback::ArraySubscriptFetcherMatchCallback(Fetcher *parent)
    : parent(parent) {
}

void ArraySubscriptFetcherMatchCallback::run(const MatchFinder::MatchResult &Result) {
    if (const auto *FS = Result.Nodes.getNodeAs<ArraySubscriptExpr>(SUBSCRIPT)) {
        SourceManager &sourceManager = Result.Context->getSourceManager();
        fs::path sourceFilePath = sourceManager.getFileEntryForID(sourceManager.getMainFileID())
                                      ->tryGetRealPathName()
                                      .str();
        auto child = FS->children().begin();
        while (canBeMissed(*child) && !child->children().empty()) {
            child = child->children().begin();
        }
        auto variableExpr = *child;
        if (!isa<DeclRefExpr>(variableExpr)) {
            return;
        }
        auto declrefExpr = cast<DeclRefExpr>(variableExpr);
        auto variableDecl = declrefExpr->getDecl();
        auto contextDecl = variableDecl->getParentFunctionOrMethod();
        if (contextDecl == nullptr || string(contextDecl->getDeclKindName()) != "Function") {
            return;
        }
        auto currentFunctionDecl = cast<FunctionDecl>(contextDecl);
        std::string functionName = currentFunctionDecl->getNameInfo().getAsString();
        auto &tests = *parent->projectTests;
        if (!CollectionUtils::containsKey(tests, sourceFilePath)) {
            return;
        }
        auto &methods = tests.at(sourceFilePath).methods;
        if (!CollectionUtils::containsKey(methods, functionName)) {
            return;
        }

        if (isa<ParmVarDecl>(variableDecl)) {
            auto paramDecl = cast<ParmVarDecl>(variableDecl);
            int paramIndex = paramDecl->getFunctionScopeIndex();
            auto &param = methods[functionName].params[paramIndex];
            param.type.maybeArray = true;
        }
        if (parent->returnVariables.count(variableDecl->getID())) {
            auto &method = methods[functionName];
            if (method.returnType.isArrayCandidate()) {
                method.returnType.maybeArray = true;
            }
        }
    }
}
