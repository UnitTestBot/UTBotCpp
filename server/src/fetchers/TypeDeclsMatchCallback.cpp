#include "TypeDeclsMatchCallback.h"

#include "Fetcher.h"
#include "clang-utils/ASTPrinter.h"
#include "clang-utils/ClangUtils.h"
#include "utils/LogUtils.h"

using namespace clang;
using namespace Matchers;

TypeDeclsMatchCallback::TypeDeclsMatchCallback(const Fetcher *parent)
    : parent(parent), typesResolver(parent) {
}

void TypeDeclsMatchCallback::run(const MatchFinder::MatchResult &Result) {
    QualType pointerType = Result.Context->getPointerType(Result.Context->getWideCharType());
    uint64_t pointerSize = Result.Context->getTypeSize(pointerType) / 8;

    ExecUtils::throwIfCancelled();
    checkStructDecl(Result);
    checkStruct(Result);
    checkEnum(Result);
    checkUnion(Result);
}

void TypeDeclsMatchCallback::checkStruct(const MatchFinder::MatchResult &Result) {
    if (const auto *ST = Result.Nodes.getNodeAs<CXXRecordDecl>(INNER_TYPEDEF_STRUCT_OR_CLASS_DECL)) {
        std::string name = ST->getNameAsString();
        if (name.empty()) {
            if (const auto *TD = Result.Nodes.getNodeAs<TypedefDecl>(TYPEDEF_STRUCT_OR_CLASS_DECL)) {
                std::string typedefName = TD->getNameAsString();
                typesResolver.resolveStruct(ST, typedefName);
            }
        }
    }

    if (const auto *ST = Result.Nodes.getNodeAs<CXXRecordDecl>(STRUCT_OR_CLASS_DECL)) {
        std::string name = ST->getNameAsString();
        typesResolver.resolveStruct(ST, name);
    }

    if (const auto *ST = Result.Nodes.getNodeAs<RecordDecl>(INNER_TYPEDEF_STRUCT_OR_CLASS_DECL)) {
        std::string name = ST->getNameAsString();
        if (name.empty()) {
            if (const auto *TD = Result.Nodes.getNodeAs<TypedefDecl>(TYPEDEF_STRUCT_OR_CLASS_DECL)) {
                std::string typedefName = TD->getNameAsString();
                typesResolver.resolveStruct(ST, typedefName);
            }
        }
    }

    if (const auto *ST = Result.Nodes.getNodeAs<RecordDecl>(STRUCT_OR_CLASS_DECL)) {
        std::string name = ST->getNameAsString();
        typesResolver.resolveStruct(ST, name);
    }
}

void TypeDeclsMatchCallback::checkStructDecl(const MatchFinder::MatchResult &Result) {
    if (const auto *ST = Result.Nodes.getNodeAs<clang::RecordDecl>(Matchers::STRUCT_OR_CLASS_JUST_DECL)) {
        fs::path sourceFilePath = ClangUtils::getSourceFilePath(Result.Context->getSourceManager());
        (*parent->structsDeclared)[sourceFilePath].insert(ST->getNameAsString());
    }
}

void TypeDeclsMatchCallback::checkEnum(const MatchFinder::MatchResult &Result) {
    if (const auto *EN = Result.Nodes.getNodeAs<EnumDecl>(INNER_TYPEDEF_ENUM_DECL)) {
        std::string name = EN->getNameAsString();
        if (name.empty()) {
            if (const auto *TD = Result.Nodes.getNodeAs<TypedefDecl>(TYPEDEF_ENUM_DECL)) {
                std::string typedefName = TD->getNameAsString();
                typesResolver.resolveEnum(EN, typedefName);
            }
        }
    }
    if (const auto *EN = Result.Nodes.getNodeAs<EnumDecl>(ENUM_DECL)) {
        std::string name = EN->getNameAsString();
        typesResolver.resolveEnum(EN, name);
    }
}

void TypeDeclsMatchCallback::checkUnion(const MatchFinder::MatchResult &Result) {
    if (const auto *ST = Result.Nodes.getNodeAs<RecordDecl>(INNER_TYPEDEF_UNION_DECL)) {
        std::string name = ST->getNameAsString();
        if (name.empty()) {
            if (const auto *TD = Result.Nodes.getNodeAs<TypedefDecl>(TYPEDEF_UNION_DECL)) {
                std::string typedefName = TD->getNameAsString();
                typesResolver.resolveUnion(ST, typedefName);
            }
        }
    }

    if (const auto *ST = Result.Nodes.getNodeAs<RecordDecl>(UNION_DECL)) {
        std::string name = ST->getNameAsString();
        typesResolver.resolveUnion(ST, name);
    }
}
