#include "ArraySubscriptFetcherMatchCallback.h"

#include "Fetcher.h"
#include "clang-utils/Matchers.h"
#include "clang-utils/ClangUtils.h"
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
        fs::path sourceFilePath = ClangUtils::getSourceFilePath(Result.Context->getSourceManager());
        auto child = FS->children().begin();
        while (canBeMissed(*child) && !child->children().empty()) {
            child = child->children().begin();
        }
        auto variableExpr = *child;
        if (!isa<DeclRefExpr>(variableExpr)) {
            return;
        }
        auto declRefExpr = cast<DeclRefExpr>(variableExpr);
        auto variableDecl = declRefExpr->getDecl();
        auto contextDecl = variableDecl->getParentFunctionOrMethod();
        if (contextDecl == nullptr || std::string(contextDecl->getDeclKindName()) != "Function") {
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
