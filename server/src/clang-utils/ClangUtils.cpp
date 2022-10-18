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
}
