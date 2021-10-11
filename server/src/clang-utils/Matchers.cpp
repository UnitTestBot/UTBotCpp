/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "Matchers.h"

using namespace clang::ast_matchers;

namespace Matchers {
    static const DeclarationMatcher TopLevelDecl =
        allOf(hasDeclContext(anyOf(namespaceDecl(), translationUnitDecl())),
              hasParent(decl(anyOf(namespaceDecl(), translationUnitDecl()))));

    AST_MATCHER(clang::TagDecl, hasIndependentType) {
        auto isDependentType = Node.getTypeForDecl()->isDependentType();
        return !isDependentType;
    }

    const DeclarationMatcher structMatcher =
        recordDecl(isStruct(), isDefinition(), hasIndependentType())
            .bind(STRUCT_DECL);

    const DeclarationMatcher classMatcher =
        recordDecl(isClass(), isDefinition(), hasIndependentType())
            .bind(CLASS_DECL);

    const DeclarationMatcher structJustDeclMatcher =
        recordDecl(isStruct())
            .bind(STRUCT_JUST_DECL);

    const DeclarationMatcher classJustDeclMatcher =
        recordDecl(isClass())
            .bind(CLASS_JUST_DECL);

    const DeclarationMatcher typedefStructMatcher =
        typedefDecl(
            hasType(
                elaboratedType(
                    namesType(
                        recordType(
                            hasDeclaration(
                                recordDecl(isStruct(), isDefinition(), hasIndependentType()).bind(INNER_TYPEDEF_STRUCT_DECL)))))))
            .bind(TYPEDEF_STRUCT_DECL);

    const DeclarationMatcher typedefClassMatcher =
        typedefDecl(
            hasType(
                elaboratedType(
                    namesType(
                        recordType(
                            hasDeclaration(
                                recordDecl(isClass(), isDefinition(), hasIndependentType()).bind(INNER_TYPEDEF_STRUCT_DECL)))))))
                  .bind(TYPEDEF_CLASS_DECL);

    const DeclarationMatcher toplevelStructMatcher =
        recordDecl(isStruct(), TopLevelDecl, hasIndependentType())
            .bind(TOPLEVEL_STRUCT_DECL);

    const DeclarationMatcher toplevelClassMatcher =
        recordDecl(isClass(), TopLevelDecl, hasIndependentType())
            .bind(TOPLEVEL_CLASS_DECL);

    const DeclarationMatcher enumMatcher =
        enumDecl(isDefinition(), hasIndependentType()).bind(ENUM_DECL);

    const DeclarationMatcher typedefEnumMatcher =
        typedefDecl(
            hasType(
                elaboratedType(
                    namesType(
                        enumType(
                            hasDeclaration(
                                enumDecl(isDefinition(), hasIndependentType())
                                    .bind(INNER_TYPEDEF_ENUM_DECL)))))))
            .bind(TYPEDEF_ENUM_DECL);
    const DeclarationMatcher toplevelEnumMatcher =
        enumDecl(TopLevelDecl, hasIndependentType())
            .bind(TOPLEVEL_ENUM_DECL);

    const DeclarationMatcher unionMatcher =
        recordDecl(isUnion(), isDefinition(), hasIndependentType()).bind(UNION_DECL);
    const DeclarationMatcher typedefUnionMatcher =
        typedefDecl(
            hasType(
                elaboratedType(
                    namesType(
                        recordType(
                            hasDeclaration(
                                recordDecl(isUnion(), isDefinition(), hasIndependentType()).bind(INNER_TYPEDEF_UNION_DECL)))))))
            .bind(TYPEDEF_UNION_DECL);
    const DeclarationMatcher toplevelUnionMatcher =
        recordDecl(isUnion(), TopLevelDecl, hasIndependentType()).bind(TOPLEVEL_UNION_DECL);

    static const auto functionDefinitionTraits = allOf(isDefinition(),
                                                       isExpansionInMainFile(),
                                                       unless(cxxDestructorDecl()),
                                                       unless(cxxConstructorDecl()),
                                                       unless(functionTemplateDecl()));

    const DeclarationMatcher functionDefinitionMatcher =
        functionDecl(functionDefinitionTraits)
            .bind(FUNCTION_DEF);

    const DeclarationMatcher anyTypeDeclarationMatcher =
        anyOf(
            structMatcher,
            classMatcher,
            typedefStructMatcher,
            typedefClassMatcher,
            enumMatcher,
            typedefEnumMatcher,
            unionMatcher,
            typedefUnionMatcher);


    const DeclarationMatcher toplevelTypedefMatcher =
        typedefDecl(TopLevelDecl).bind(TOPLEVEL_TYPEDEF);

    const DeclarationMatcher toplevelVarMatcher =
        varDecl(TopLevelDecl, unless(isExpansionInSystemHeader())).bind(TOPLEVEL_VAR_DECL);

    const DeclarationMatcher anyToplevelTypeDeclarationMatcher =
        anyOf(
            toplevelStructMatcher,
            toplevelClassMatcher,
            toplevelEnumMatcher,
            toplevelUnionMatcher,
            toplevelTypedefMatcher,
            toplevelVarMatcher);

    const DeclarationMatcher anyToplevelDeclarationMatcher = anyOf(
        anyToplevelTypeDeclarationMatcher,
        functionDefinitionMatcher);

    const DeclarationMatcher globalVariableUsageMatcher = functionDecl(functionDefinitionTraits,
        forEachDescendant(declRefExpr(
            to(
                varDecl(
                    hasGlobalStorage(), unless(hasAncestor(functionDecl()))
                ).bind(GLOBAL_VARIABLE_USAGE)
            )
        ))
    ).bind(FUNCTION_USED_GLOBAL_VARIABLE);

    const StatementMatcher arraySubscriptMatcher = arraySubscriptExpr(isExpansionInMainFile()).bind(SUBSCRIPT);
    const StatementMatcher returnMatcher = returnStmt(isExpansionInMainFile()).bind(RETURN);
}