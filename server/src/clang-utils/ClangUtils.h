/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_CLANGUTILS_H
#define UNITTESTBOT_CLANGUTILS_H

#include "clang-utils/Matchers.h"
#include <clang/AST/Type.h>

namespace ClangUtils {
    bool isIncomplete(clang::QualType type);
    const clang::CXXConstructorDecl *isConstructor(const clang::ast_matchers::MatchFinder::MatchResult &Result);
    const clang::FunctionDecl *isFunctionOrConstructor(const clang::ast_matchers::MatchFinder::MatchResult &Result);
    clang::QualType getReturnType(const clang::FunctionDecl *FS, const clang::ast_matchers::MatchFinder::MatchResult &Result);
};


#endif // UNITTESTBOT_CLANGUTILS_H
