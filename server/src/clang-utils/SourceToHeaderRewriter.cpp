#include "SourceToHeaderRewriter.h"

#include "NameDecorator.h"
#include "SettingsContext.h"
#include "SourceToHeaderMatchCallback.h"
#include "utils/Copyright.h"
#include "utils/LogUtils.h"

#include "loguru.h"

#include <utility>
#include <fstream>

SourceToHeaderRewriter::SourceToHeaderRewriter(
    utbot::ProjectContext projectContext,
    const std::shared_ptr<CompilationDatabase> &compilationDatabase,
    std::shared_ptr<Fetcher::FileToStringSet> structsToDeclare,
    fs::path serverBuildDir,
    const types::TypesHandler &typesHandler)
    : projectContext(std::move(projectContext)),
      clangToolRunner(compilationDatabase), structsToDeclare(structsToDeclare),
      serverBuildDir(std::move(serverBuildDir)), typesHandler(typesHandler) {
}

std::unique_ptr<clang::tooling::FrontendActionFactory>
SourceToHeaderRewriter::createFactory(llvm::raw_ostream *externalStream,
                                      llvm::raw_ostream *internalStream,
                                      llvm::raw_ostream *unnamedTypeDeclsStream,
                                      llvm::raw_ostream *wrapperStream,
                                      fs::path sourceFilePath,
                                      bool forStubHeader) {
    if (Paths::isCXXFile(sourceFilePath)) {
        externalStream = nullptr;
        internalStream = nullptr;
    }
    fetcherInstance = std::make_unique<SourceToHeaderMatchCallback>(
        projectContext, sourceFilePath, externalStream, internalStream, unnamedTypeDeclsStream, wrapperStream, typesHandler, forStubHeader);
    finder = std::make_unique<clang::ast_matchers::MatchFinder>();
    finder->addMatcher(Matchers::anyToplevelDeclarationMatcher, fetcherInstance.get());
    return clang::tooling::newFrontendActionFactory(finder.get());
}

SourceToHeaderRewriter::SourceDeclarations
SourceToHeaderRewriter::generateSourceDeclarations(const fs::path &sourceFilePath, bool forStubHeader) {
    std::string externalDeclarations;
    llvm::raw_string_ostream externalStream(externalDeclarations);
    std::string internalDeclarations;
    llvm::raw_string_ostream internalStream(internalDeclarations);
    std::string unnamedTypeDeclarations;
    llvm::raw_string_ostream unnamedTypeDeclsStream(unnamedTypeDeclarations);

    auto factory = createFactory(&externalStream, &internalStream, &unnamedTypeDeclsStream, nullptr, sourceFilePath, forStubHeader);

    if (CollectionUtils::containsKey(*structsToDeclare, sourceFilePath)) {
        std::stringstream newContentStream;
        for (std::string const &structName : structsToDeclare->at(sourceFilePath)) {
            newContentStream << StringUtils::stringFormat("struct %s;\n", structName);
        }
        std::ifstream oldFileStream(sourceFilePath);
        newContentStream << oldFileStream.rdbuf();
        std::string content = newContentStream.str();
        clangToolRunner.run(sourceFilePath, factory.get(), false, content);
    } else {
        clangToolRunner.run(sourceFilePath, factory.get());
    }
    externalStream.flush();
    internalStream.flush();
    unnamedTypeDeclsStream.flush();

    return { externalDeclarations, internalDeclarations, unnamedTypeDeclarations };
}


std::string SourceToHeaderRewriter::generateTestHeader(const fs::path &sourceFilePath,
                                                       const Tests &test) {
    MEASURE_FUNCTION_EXECUTION_TIME
    auto sourceDeclarations = generateSourceDeclarations(sourceFilePath, false);

    if (Paths::isCXXFile(sourceFilePath)) {
        auto sourceFileToInclude = sourceFilePath;
        if (test.mainHeader.has_value()) {
            sourceFileToInclude =
                fs::canonical(test.sourceFilePath.parent_path() / test.mainHeader.value().path);
        }
        sourceFileToInclude = fs::relative(sourceFilePath, test.testHeaderFilePath.parent_path());
        return StringUtils::stringFormat("#define main main__\n\n"
                                         "#include \"%s\"\n\n"
                                         "%s\n",
                                         sourceFileToInclude, sourceDeclarations.unnamedTypeDeclarations);
    }

    return StringUtils::stringFormat(
        "%s\n"
        "namespace %s {\n"
        "%s\n"
        "%s\n"
        "%s\n"
        "%s\n"
        "%s\n"
        "%s\n"
        "%s\n"
        "\n%s"
        "}\n",
        Copyright::GENERATED_C_CPP_FILE_HEADER, PrinterUtils::TEST_NAMESPACE,
        NameDecorator::DEFINES_CODE, PrinterUtils::DEFINES_FOR_C_KEYWORDS,
        PrinterUtils::KNOWN_IMPLICIT_RECORD_DECLS_CODE,
        sourceDeclarations.externalDeclarations, sourceDeclarations.internalDeclarations,
        NameDecorator::UNDEF_WCHAR_T, NameDecorator::UNDEFS_CODE, sourceDeclarations.unnamedTypeDeclarations);
}

std::string SourceToHeaderRewriter::generateStubHeader(const fs::path &sourceFilePath) {
    MEASURE_FUNCTION_EXECUTION_TIME
    LOG_IF_S(WARNING, Paths::isCXXFile(sourceFilePath))
        << "Stubs feature for C++ sources has not been tested thoroughly; some problems may occur";
    auto sourceDeclarations = generateSourceDeclarations(sourceFilePath, true);
    long long creationTime = TimeUtils::convertFileToSystemClock(fs::file_time_type::clock::now())
                                 .time_since_epoch()
                                 .count();
    return StringUtils::stringFormat(
        "//%s\n"
        "//Please, do not change the line above\n"
        "%s\n"
        "#define _Alignas(x)\n"
        "%s\n",
        std::to_string(creationTime), Copyright::GENERATED_C_CPP_FILE_HEADER,
        sourceDeclarations.externalDeclarations);
}

std::string SourceToHeaderRewriter::generateWrapper(const fs::path &sourceFilePath) {
    MEASURE_FUNCTION_EXECUTION_TIME
    if (!Paths::isCFile(sourceFilePath)) {
        return "";
    }
    std::string result;
    llvm::raw_string_ostream wrapperStream(result);
    auto factory = createFactory(nullptr, nullptr, nullptr, &wrapperStream, sourceFilePath, false);
    clangToolRunner.run(sourceFilePath, factory.get());
    wrapperStream.flush();
    return result;
}

void SourceToHeaderRewriter::generateTestHeaders(tests::TestsMap &tests,
                                             ProgressWriter const *progressWriter) {
    std::string logMessage = "Generating headers for tests";
    LOG_S(DEBUG) << logMessage;
    ExecUtils::doWorkWithProgress(tests, progressWriter, logMessage, [this](auto &it) {
        fs::path const &sourceFilePath = it.first;
        tests::Tests &test = const_cast<tests::Tests &>(it.second);
        test.headerCode = generateTestHeader(sourceFilePath, test);
    });
}
