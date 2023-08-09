#ifndef UNITTESTBOT_SOURCETOHEADERREWRITER_H
#define UNITTESTBOT_SOURCETOHEADERREWRITER_H

#include "Paths.h"
#include "SettingsContext.h"
#include "SimpleFrontendActionFactory.h"
#include "Tests.h"
#include "building/BuildDatabase.h"
#include "fetchers/Fetcher.h"
#include "fetchers/FetcherUtils.h"
#include "stubs/StubGen.h"
#include "utils/FileSystemUtils.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <grpcpp/grpcpp.h>

#include <utility>

class SourceToHeaderRewriter {
    const utbot::ProjectContext projectContext;

    ClangToolRunner clangToolRunner;
    fs::path projectPath;
    fs::path serverBuildDir;
    std::shared_ptr<Fetcher::FileToStringSet> structsToDeclare;
    const types::TypesHandler &typesHandler;

    std::unique_ptr<clang::ast_matchers::MatchFinder::MatchCallback> fetcherInstance;
    std::unique_ptr<clang::ast_matchers::MatchFinder> finder;

    std::unique_ptr<clang::tooling::FrontendActionFactory>
    createFactory(llvm::raw_ostream *externalStream,
                  llvm::raw_ostream *internalStream,
                  llvm::raw_ostream *unnamedTypeDeclsStream,
                  llvm::raw_ostream *wrapperStream,
                  fs::path sourceFilePath,
                  bool forStubHeader,
                  bool externFromStub);

public:
    struct SourceDeclarations {
        std::string externalDeclarations;
        std::string internalDeclarations;
        std::string unnamedTypeDeclarations;
    };

    friend class SourceToHeaderMatchCallback;

    SourceToHeaderRewriter(
        utbot::ProjectContext projectContext,
        const std::shared_ptr<CompilationDatabase> &compilationDatabase,
        std::shared_ptr<Fetcher::FileToStringSet> structsToDeclare,
        fs::path serverBuildDir,
        const types::TypesHandler &typesHandler);

    SourceDeclarations generateSourceDeclarations(const fs::path &sourceFilePath, bool forStubHeader, bool externFromStub);

    std::string generateTestHeader(const fs::path &sourceFilePath, const Tests &test, bool externFromStub);

    std::string generateStubHeader(const fs::path &sourceFilePath);

    std::string generateWrapper(const fs::path &sourceFilePath);

    void generateTestHeaders(tests::TestsMap &tests,
                             const StubGen &stubGen,
                             const CollectionUtils::MapFileTo<fs::path> &selectedTargets,
                             ProgressWriter const *progressWriter);
};


#endif // UNITTESTBOT_SOURCETOHEADERREWRITER_H
