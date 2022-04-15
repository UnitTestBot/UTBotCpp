/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_MATCHERS_H
#define UNITTESTBOT_MATCHERS_H

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>

#include <string>

namespace Matchers {
    using DeclarationMatcher = clang::ast_matchers::DeclarationMatcher;
    using StatementMatcher = clang::ast_matchers::StatementMatcher;

    static inline const std::string STRUCT_OR_CLASS_DECL = "struct_decl";
    static inline const std::string STRUCT_OR_CLASS_JUST_DECL = "struct_just_decl";
    static inline const std::string TOPLEVEL_STRUCT_OR_CLASS_DECL = "toplevel_struct_decl";
    static inline const std::string INNER_TYPEDEF_STRUCT_OR_CLASS_DECL = "the_struct";
    static inline const std::string TYPEDEF_STRUCT_OR_CLASS_DECL = "the_struct_typedef";

    static inline const std::string ENUM_DECL = "enum_decl";
    static inline const std::string TOPLEVEL_ENUM_DECL = "toplevel_enum_decl";
    static inline const std::string INNER_TYPEDEF_ENUM_DECL = "the_enum";
    static inline const std::string TYPEDEF_ENUM_DECL = "the_enum_typedef";

    static inline const std::string UNION_DECL = "union_decl";
    static inline const std::string INNER_TYPEDEF_UNION_DECL = "the_union";
    static inline const std::string TOPLEVEL_UNION_DECL = "toplevel_union_decl";
    static inline const std::string TYPEDEF_UNION_DECL = "the_union_typedef";

    static inline const std::string FUNCTION_DEF = "function_def";
    static inline const std::string CONSTRUCTOR_DEF = "constructor_def";
    static inline const std::string TOPLEVEL_TYPEDEF = "typedef_decl";

    static inline const std::string TOPLEVEL_VAR_DECL = "toplevel_var_decl";

    static inline const std::string FUNCTION_USED_GLOBAL_VARIABLE = "function_used_global_variable";
    static inline const std::string GLOBAL_VARIABLE_USAGE = "global_varaiable_usage";

    static inline const std::string SUBSCRIPT = "subscript";
    static inline const std::string RETURN = "return";

    extern const DeclarationMatcher functionDefinitionMatcher;
    extern const DeclarationMatcher constructorDefinitionMatcher;
    extern const DeclarationMatcher memberConstructorDefinitionMatcher;

    extern const DeclarationMatcher structMatcher;
    extern const DeclarationMatcher structJustDeclMatcher;
    extern const DeclarationMatcher typedefStructMatcher;
    extern const DeclarationMatcher toplevelStructMatcher;

    extern const DeclarationMatcher classMatcher;
    extern const DeclarationMatcher classJustDeclMatcher;
    extern const DeclarationMatcher typedefClassMatcher;
    extern const DeclarationMatcher toplevelClassMatcher;

    extern const DeclarationMatcher enumMatcher;
    extern const DeclarationMatcher typedefEnumMatcher;
    extern const DeclarationMatcher toplevelEnumMatcher;

    extern const DeclarationMatcher unionMatcher;
    extern const DeclarationMatcher typedefUnionMatcher;
    extern const DeclarationMatcher toplevelUnionMatcher;

    extern const DeclarationMatcher anyTypeDeclarationMatcher;
    extern const DeclarationMatcher anyToplevelTypeDeclarationMatcher;
    extern const DeclarationMatcher anyToplevelDeclarationMatcher;

    extern const DeclarationMatcher globalVariableUsageMatcher;

    extern const StatementMatcher arraySubscriptMatcher;
    extern const StatementMatcher returnMatcher;
}


#endif // UNITTESTBOT_MATCHERS_H
