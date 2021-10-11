/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "AbstractType.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"

#include <iostream>


class TypeVisitor : public clang::RecursiveASTVisitor<TypeVisitor> {
public:
    // Warning 'function TraverseType hides a non-virtual function from class RecursiveASTVisitor<TypeVisitor>`
    // is totally fine.
    bool TraverseType(clang::QualType type);

    std::vector<std::shared_ptr<AbstractType>> getKinds();

    std::vector<std::string> getTypes();

private:
    std::vector<std::string> types{};
    std::vector<std::shared_ptr<AbstractType>> kinds{};
};
