/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

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
