/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_CLANGUTILS_H
#define UNITTESTBOT_CLANGUTILS_H

#include <clang/AST/Type.h>

namespace ClangUtils {
    bool isPointerToIncomplete(clang::QualType type);
};


#endif // UNITTESTBOT_CLANGUTILS_H
