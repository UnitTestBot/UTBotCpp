/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "ClangUtils.h"

#include <clang/AST/Decl.h>

namespace ClangUtils {
    static bool isIncompleteTagDecl(clang::QualType type) {
        if (!type.isNull()) {
            if (auto const *tagDecl = type->getAsTagDecl()) {
                return tagDecl->getDefinition() == nullptr;
            }
        }
        return false;
    }

    bool isIncomplete(clang::QualType type) {
        clang::QualType canonicalType = type.getCanonicalType();
        if (auto const *pType = canonicalType.getTypePtrOrNull()) {
            auto pointeeType = pType->getPointeeType();
            if (!pointeeType.isNull()) {
                return isIncompleteTagDecl(pointeeType) ||
                       isIncompleteTagDecl(pointeeType->getPointeeType());
            }
        }
        return false;
    }

    const clang::CXXConstructorDecl *isConstructor(const clang::ast_matchers::MatchFinder::MatchResult &Result) {
        return Result.Nodes.getNodeAs<clang::CXXConstructorDecl>(Matchers::CONSTRUCTOR_DEF);
    }

    const clang::FunctionDecl *isFunctionOrConstructor(const clang::ast_matchers::MatchFinder::MatchResult &Result) {
        const auto *FS = Result.Nodes.getNodeAs<clang::FunctionDecl>(Matchers::FUNCTION_DEF);
        if (!FS) {
            FS = isConstructor(Result);
        }
        return FS;
    }


    clang::QualType getReturnType(const clang::FunctionDecl *FS, const clang::ast_matchers::MatchFinder::MatchResult &Result) {
        clang::QualType realReturnType = FS->getReturnType().getCanonicalType();
        if (const auto *CS = isConstructor(Result)) {
            realReturnType = CS->getThisObjectType();
        }
        return realReturnType;
    }
}
