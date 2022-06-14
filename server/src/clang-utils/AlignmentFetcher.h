#ifndef UNITTESTBOT_ALIGNMENTFETCHER_H
#define UNITTESTBOT_ALIGNMENTFETCHER_H

#include <clang/AST/Decl.h>

#include <optional>

class AlignmentFetcher {
public:
    static std::optional<uint64_t> fetch(const clang::VarDecl *parmVarDecl);

private:
    static uint64_t handleDecl(const clang::Decl *decl);
};


#endif // UNITTESTBOT_ALIGNMENTFETCHER_H
