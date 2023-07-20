#ifndef UNITTESTBOT_TYPESRESOLVER_H
#define UNITTESTBOT_TYPESRESOLVER_H

#include "types/Types.h"

#include <clang/AST/Decl.h>

#include <cstddef>
#include <string>
#include <utility>

class Fetcher;

class TypesResolver {
private:
    const Fetcher *const parent;
    std::map <uint64_t, std::string> fullname;
    std::map <uint64_t, std::string> fieldName;
public:
    explicit TypesResolver(Fetcher const *parent);

    void resolveStruct(const clang::RecordDecl *D, const std::string &name);

    void resolveEnum(const clang::EnumDecl *EN, const std::string &name);

    void resolveUnion(const clang::RecordDecl *D, const std::string &name);

    void resolveStructEx(const clang::RecordDecl *D,
                         const std::string &name,
                         types::SubType subType);

    void resolve(const clang::QualType &type);

private:
    std::string getFullname(const clang::TagDecl *TD, const clang::QualType &canonicalType,
                            uint64_t id, const fs::path &sourceFilePath);

    void updateMaximumAlignment(size_t alignment) const;
};


#endif // UNITTESTBOT_TYPESRESOLVER_H
