#ifndef UNITTESTBOT_CLANGUTILS_H
#define UNITTESTBOT_CLANGUTILS_H

#include <clang/AST/Type.h>

namespace ClangUtils {
    bool isIncomplete(clang::QualType type);
};


#endif // UNITTESTBOT_CLANGUTILS_H
