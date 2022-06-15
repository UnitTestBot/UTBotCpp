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

    const clang::CXXConstructorDecl *getConstructor(const clang::ast_matchers::MatchFinder::MatchResult &Result) {
        return Result.Nodes.getNodeAs<clang::CXXConstructorDecl>(Matchers::CONSTRUCTOR_DEF);
    }

    const clang::FunctionDecl *getFunctionOrConstructor(const clang::ast_matchers::MatchFinder::MatchResult &Result) {
        const auto *FS = Result.Nodes.getNodeAs<clang::FunctionDecl>(Matchers::FUNCTION_DEF);
        if (!FS) {
            FS = getConstructor(Result);
        }
        return FS;
    }


    clang::QualType getReturnType(const clang::FunctionDecl *FS, const clang::ast_matchers::MatchFinder::MatchResult &Result) {
        clang::QualType realReturnType = FS->getReturnType().getCanonicalType();
        if (const auto *CS = getConstructor(Result)) {
            realReturnType = CS->getThisObjectType();
        }
        return realReturnType;
    }
}
