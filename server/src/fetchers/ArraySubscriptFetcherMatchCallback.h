#ifndef UNITTESTBOT_ARRAYSUBSCRIPTFETCHERMATCHCALLBACK_H
#define UNITTESTBOT_ARRAYSUBSCRIPTFETCHERMATCHCALLBACK_H

#include <clang/ASTMatchers/ASTMatchFinder.h>

class Fetcher;

class ArraySubscriptFetcherMatchCallback : public clang::ast_matchers::MatchFinder::MatchCallback {
    using MatchFinder = clang::ast_matchers::MatchFinder;
public:
    explicit ArraySubscriptFetcherMatchCallback(Fetcher *parent);

    void run(const MatchFinder::MatchResult &Result) override;

private:
    Fetcher *parent;
};


#endif // UNITTESTBOT_ARRAYSUBSCRIPTFETCHERMATCHCALLBACK_H
