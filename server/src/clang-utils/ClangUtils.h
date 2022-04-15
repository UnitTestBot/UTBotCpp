#ifndef UNITTESTBOT_CLANGUTILS_H
#define UNITTESTBOT_CLANGUTILS_H

#include "clang-utils/Matchers.h"
#include <clang/AST/Type.h>

namespace ClangUtils {
    bool isIncomplete(clang::QualType type);

    const clang::CXXConstructorDecl *getConstructor(const clang::ast_matchers::MatchFinder::MatchResult &Result);

    const clang::FunctionDecl *getFunctionOrConstructor(const clang::ast_matchers::MatchFinder::MatchResult &Result);

    clang::QualType
    getReturnType(const clang::FunctionDecl *FS, const clang::ast_matchers::MatchFinder::MatchResult &Result);

    /*
     * @return commands with qualifier if needed for call it in tests
     */
    std::string getCallName(const clang::FunctionDecl *FS);

    inline std::string getSourceFilePath(const clang::SourceManager &sourceManager) {
        return sourceManager.getFileEntryForID(sourceManager.getMainFileID())->tryGetRealPathName().str();
    }
};


#endif // UNITTESTBOT_CLANGUTILS_H
