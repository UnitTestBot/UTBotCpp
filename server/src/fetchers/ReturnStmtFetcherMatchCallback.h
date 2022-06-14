#ifndef UNITTESTBOT_RETURNSTMTFETCHERMATCHCALLBACK_H
#define UNITTESTBOT_RETURNSTMTFETCHERMATCHCALLBACK_H

#include <clang/ASTMatchers/ASTMatchFinder.h>

class Fetcher;

class ReturnStmtFetcherMatchCallback : public clang::ast_matchers::MatchFinder::MatchCallback {
    using MatchFinder = clang::ast_matchers::MatchFinder;
public:
    explicit ReturnStmtFetcherMatchCallback(Fetcher *parent);

    void run(const MatchFinder::MatchResult &Result) override;

private:
    Fetcher *parent;
};


#endif // UNITTESTBOT_RETURNSTMTFETCHERMATCHCALLBACK_H
