/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_TYPEDECLSMATCHCALLBACK_H
#define UNITTESTBOT_TYPEDECLSMATCHCALLBACK_H

#include "types/TypesResolver.h"

#include "loguru.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>

#include <unordered_map>

class Fetcher;

class TypeDeclsMatchCallback : public clang::ast_matchers::MatchFinder::MatchCallback {
    using MatchFinder = clang::ast_matchers::MatchFinder;

public:
    explicit TypeDeclsMatchCallback(Fetcher const *parent);

    void run(const MatchFinder::MatchResult &Result) override;

    void checkStruct(const MatchFinder::MatchResult &Result);

    void checkStructDecl(const MatchFinder::MatchResult &Result);

    void checkEnum(const MatchFinder::MatchResult &Result);

    void checkUnion(const MatchFinder::MatchResult &Result);

private:
    void handleStruct(const MatchFinder::MatchResult &Result,
                      const clang::RecordDecl *D,
                      const std::string &name);

    void handleEnum(const MatchFinder::MatchResult &Result,
                    const clang::EnumDecl *EN,
                    const std::string &name);

    void handleUnion(const MatchFinder::MatchResult &Result,
                     const clang::RecordDecl *D,
                     const std::string &name);

private:
    Fetcher const *const parent;
    TypesResolver typesResolver;
};


#endif // UNITTESTBOT_TYPEDECLSMATCHCALLBACK_H
