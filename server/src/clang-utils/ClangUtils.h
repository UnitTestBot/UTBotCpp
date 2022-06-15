#ifndef UNITTESTBOT_CLANGUTILS_H
#define UNITTESTBOT_CLANGUTILS_H

#include "clang-utils/Matchers.h"
#include <clang/AST/Type.h>

namespace ClangUtils {
    bool isIncomplete(clang::QualType type);
    const clang::CXXConstructorDecl *getConstructor(const clang::ast_matchers::MatchFinder::MatchResult &Result);
    const clang::FunctionDecl *getFunctionOrConstructor(const clang::ast_matchers::MatchFinder::MatchResult &Result);
    clang::QualType getReturnType(const clang::FunctionDecl *FS, const clang::ast_matchers::MatchFinder::MatchResult &Result);
};


#endif // UNITTESTBOT_CLANGUTILS_H
