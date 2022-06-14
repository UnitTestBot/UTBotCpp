#ifndef UNITTESTBOT_GLOBALVARIABLEUSAGEMATCHCALLBACK_H
#define UNITTESTBOT_GLOBALVARIABLEUSAGEMATCHCALLBACK_H

#include "Fetcher.h"
#include "FetcherUtils.h"
#include "Fetcher.h"

#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <grpcpp/grpcpp.h>

class Fetcher;

class GlobalVariableUsageMatchCallback : public clang::ast_matchers::MatchFinder::MatchCallback {
    using MatchFinder = clang::ast_matchers::MatchFinder;

public:
    explicit GlobalVariableUsageMatchCallback(const Fetcher *parent)
        : parent(parent) {
    }

    void run(const MatchFinder::MatchResult &Result) override;

private:
    void checkUsage(const MatchFinder::MatchResult &Result);

    void handleUsage(const clang::FunctionDecl *functionDecl, const clang::VarDecl *varDecl);

    Fetcher const *const parent;

    struct Usage {
        std::string variableName;
        std::string functionName;

        Usage(std::string variableName, std::string functionName);

        bool operator==(const Usage &rhs) const;
        bool operator!=(const Usage &rhs) const;
    };

    struct UsageHash {
        std::size_t operator()(Usage const &usage) const;
    };
    std::unordered_set<Usage, UsageHash> usages;
};

#endif // UNITTESTBOT_GLOBALVARIABLEUSAGEMATCHCALLBACK_H
