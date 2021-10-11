/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_TYPESRESOLVER_H
#define UNITTESTBOT_TYPESRESOLVER_H

#include "types/Types.h"

#include <clang/AST/Decl.h>

#include <string>
#include <utility>

class Fetcher;

class TypesResolver {
private:
    const Fetcher *const parent;
    const utbot::Language srcLanguage;
public:
    explicit TypesResolver(Fetcher const *parent, utbot::Language srcLanguage = utbot::Language::C);

    void resolveStruct(const clang::RecordDecl *D, const std::string &name);

    void resolveEnum(const clang::EnumDecl *EN, const std::string &name);

    void resolveUnion(const clang::RecordDecl *D, const std::string &name);

    void resolve(const clang::QualType &type);

private:
    void updateMaximumAlignment(uint64_t alignment) const;
};


#endif // UNITTESTBOT_TYPESRESOLVER_H
