/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "AlignmentFetcher.h"

#include <clang/AST/Attr.h>

using namespace clang;

std::optional<uint64_t> AlignmentFetcher::fetch(const clang::VarDecl *parmVarDecl) {
    uint64_t alignment = parmVarDecl->getMaxAlignment();
    alignment = std::max(alignment, handleDecl(parmVarDecl));
    if (parmVarDecl->getType()->isVoidPointerType()) {
        return std::max<uint64_t>(alignment, 0ULL);
    }
    if (alignment != 0) {
        return alignment;
    }
    return std::nullopt;
}

uint64_t AlignmentFetcher::handleDecl(const Decl *decl) {
    uint64_t alignment = 0;
    if (const auto *pAlignValueAttr = decl->getAttr<AlignValueAttr>()) {
        uint64_t value = pAlignValueAttr->getAlignment()
                             ->EvaluateKnownConstInt(decl->getASTContext())
                             .getZExtValue();
        alignment = std::max(alignment, value);
    }
    return alignment;
}
