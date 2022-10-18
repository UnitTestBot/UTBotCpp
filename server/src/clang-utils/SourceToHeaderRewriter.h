#ifndef UNITTESTBOT_SOURCETOHEADERREWRITER_H
#define UNITTESTBOT_SOURCETOHEADERREWRITER_H

#include "Paths.h"
#include "SettingsContext.h"
#include "SimpleFrontendActionFactory.h"
#include "Tests.h"
#include "building/BuildDatabase.h"
#include "fetchers/Fetcher.h"
#include "fetchers/FetcherUtils.h"
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

    std::unique_ptr<clang::ast_matchers::MatchFinder::MatchCallback> fetcherInstance;
    std::unique_ptr<clang::ast_matchers::MatchFinder> finder;

    std::unique_ptr<clang::tooling::FrontendActionFactory>
    createFactory(llvm::raw_ostream *externalStream,
                  llvm::raw_ostream *internalStream,
                  llvm::raw_ostream *wrapperStream,
                  fs::path sourceFilePath,
                  bool forStubHeader);

public:
    struct SourceDeclarations {
        std::string externalDeclarations;
        std::string internalDeclarations;
    };

    friend class SourceToHeaderMatchCallback;

    SourceToHeaderRewriter(
        utbot::ProjectContext projectContext,
        const std::shared_ptr<CompilationDatabase> &compilationDatabase,
        std::shared_ptr<Fetcher::FileToStringSet> structsToDeclare,
        fs::path serverBuildDir);

    SourceDeclarations generateSourceDeclarations(const fs::path &sourceFilePath, bool forStubHeader);

    std::string generateTestHeader(const fs::path &sourceFilePath, const Tests &test);

    std::string generateStubHeader(const fs::path &sourceFilePath);

    std::string generateWrapper(const fs::path &sourceFilePath);

    void generateTestHeaders(tests::TestsMap &tests, ProgressWriter const *progressWriter);
};


#endif // UNITTESTBOT_SOURCETOHEADERREWRITER_H
