#ifndef UNITTESTBOT_SOURCETOHEADERMATCHCALLBACK_H
#define UNITTESTBOT_SOURCETOHEADERMATCHCALLBACK_H

#include "ProjectContext.h"
#include "SettingsContext.h"
#include "fetchers/Fetcher.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <protobuf/testgen.grpc.pb.h>

#include "utils/path/FileSystemPath.h"
#include <string>
#include <unordered_set>

class SourceToHeaderRewriter;

class SourceToHeaderMatchCallback : public clang::ast_matchers::MatchFinder::MatchCallback {
    using MatchFinder = clang::ast_matchers::MatchFinder;

    utbot::ProjectContext projectContext;
    fs::path sourceFilePath;
    llvm::raw_ostream *const externalStream = nullptr;
    llvm::raw_ostream *const internalStream = nullptr;
    llvm::raw_ostream *const unnamedTypeDeclsStream = nullptr;
    llvm::raw_ostream *const wrapperStream = nullptr;

    std::unordered_set<std::string> variables{};

    const types::TypesHandler &typesHandler;

    bool forStubHeader;
    bool externFromStub;
public:
    SourceToHeaderMatchCallback(
                                utbot::ProjectContext projectContext,
                                fs::path sourceFilePath,
                                llvm::raw_ostream *externalStream,
                                llvm::raw_ostream *internalStream,
                                llvm::raw_ostream *unnamedTypeDeclsStream,
                                llvm::raw_ostream *wrapperStream,
                                const types::TypesHandler &typesHandler,
                                bool forStubHeader,
                                bool externFromStub);

    void run(const MatchFinder::MatchResult &Result) override;
private:
    void checkStruct(const MatchFinder::MatchResult &Result);

    void checkEnum(const MatchFinder::MatchResult &Result);

    void checkUnion(const MatchFinder::MatchResult &Result);

    void checkTypedef(const MatchFinder::MatchResult &Result);

    void checkFunctionDecl(const MatchFinder::MatchResult &Result);

    void checkVarDecl(const MatchFinder::MatchResult &Result);

    void handleStruct(const clang::RecordDecl *decl);

    void handleEnum(const clang::EnumDecl *decl);

    void handleUnion(const clang::RecordDecl *decl);

    void handleTypedef(const clang::TypedefDecl *decl);

    void handleFunctionDecl(const clang::FunctionDecl *decl);

    void handleVarDecl(const clang::VarDecl *decl);

    void print(const clang::NamedDecl *decl, const clang::PrintingPolicy &policy) const;

    void print(const clang::NamedDecl *decl) const;

    clang::PrintingPolicy getDefaultPrintingPolicy(const clang::Decl *decl,
                                                          bool adjustForCPlusPlus) const;

    void printReturn(const clang::FunctionDecl *decl,
                     std::string const &name,
                     llvm::raw_ostream *stream) const;

    void printUnnamedTypeDecl(const std::string &structName,
                              const std::string &fieldName,
                              const std::string &typeName) const;

    void generateWrapper(const clang::FunctionDecl *decl) const;

    void generateWrapper(const clang::VarDecl *decl) const;

    void generateInternal(const clang::FunctionDecl *decl) const;

    void generateInternal(const clang::VarDecl *decl) const;

    void generateUnnamedTypeDeclsForFields(const types::StructInfo &info) const;

    void generateUnnamedTypeDecls(const clang::RecordDecl *decl) const;

    std::string generateTypedefForGetterReturnType(const clang::VarDecl *decl,
                                                   const clang::PrintingPolicy &policy,
                                                   const std::string &returnTypeName) const;

    std::string getRenamedDeclarationAsString(const clang::NamedDecl *decl,
                                              clang::PrintingPolicy const &policy,
                                              std::string const &name) const;

    void renameDecl(const clang::NamedDecl *decl, const std::string &name) const;

    void renameAnonymousReturnTypeDecl(const clang::TagDecl *tagDecl, const std::string &methodName) const;

    std::string decorate(std::string_view name) const;

    void replaceAnonymousEnumTypeName(std::string &strDecl,
                                      const std::string &typeName) const;

    bool isAnonymousEnumDecl(const clang::TagDecl *tagDecl) const;

    std::string getOldStyleDeclarationAsString(const clang::FunctionDecl *decl, std::string const &name) const;

    bool IsOldStyleDefinition(std::string const &definition) const;
};


#endif // UNITTESTBOT_SOURCETOHEADERMATCHCALLBACK_H
