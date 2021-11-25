/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_FUNCTIONDECLSMATCHCALLBACK_H
#define UNITTESTBOT_FUNCTIONDECLSMATCHCALLBACK_H

#include "Tests.h"
#include "types/TypesResolver.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>

class Fetcher;

class FunctionDeclsMatchCallback : public clang::ast_matchers::MatchFinder::MatchCallback {
    using MatchFinder = clang::ast_matchers::MatchFinder;

public:
    explicit FunctionDeclsMatchCallback(const Fetcher *parent,
                                        bool onlyNames,
                                        bool toResolveReturnTypes,
                                        bool onlyReturnTypes);

    void run(const MatchFinder::MatchResult &Result) override;

private:
    Fetcher const *const parent;
    TypesResolver typesResolver;
    bool onlyNames;
    bool toResolveReturnTypes;
    bool onlyReturnTypes;

    using MethodsSet =
        std::unordered_set<tests::Tests::MethodDescription, tests::Tests::MethodDescriptionHash>;
    CollectionUtils::MapFileTo<MethodsSet> methods{};

    static void logFunction(const tests::Tests::MethodDescription &description,
                            const fs::path &path);
    void addMethod(const fs::path &sourceFilePath,
                   const tests::Tests::MethodDescription &methodDescription);

    void addFunctionPointer(tests::Tests::MethodDescription::FPointerMap &functionPointers,
                       const clang::FunctionType *functionType,
                       const clang::QualType &qualType,
                       const string &name,
                       const clang::SourceManager &sourceManager,
                       const types::Type &type);
};


#endif // UNITTESTBOT_FUNCTIONDECLSMATCHCALLBACK_H
