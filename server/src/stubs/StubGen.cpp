#include "StubGen.h"

#include "FeaturesFilter.h"
#include "Paths.h"
#include "StubSourcesFinder.h"
#include "Synchronizer.h"
#include "TimeExecStatistics.h"
#include "clang-utils/SourceToHeaderRewriter.h"
#include "printers/CCJsonPrinter.h"
#include "streams/stubs/StubsWriter.h"
#include "loguru.h"
#include "environment/EnvironmentPaths.h"

StubGen::StubGen(BaseTestGen &testGen) : testGen(testGen) {
}

CollectionUtils::FileSet StubGen::getStubSources(const fs::path &target) const {
    if (!testGen.needToBeMocked() || !testGen.settingsContext.useStubs) {
        return {};
    }
    fs::path testedFilePath = *testGen.testingMethodsSourcePaths.begin();
    auto stubSources = StubSourcesFinder(testGen.getProjectBuildDatabase()).excludeFind(testedFilePath, target);
    return { stubSources.begin(), stubSources.end() };
}

CollectionUtils::FileSet
StubGen::findStubFilesBySignatures(const std::vector<tests::Tests::MethodDescription> &signatures) {
    fs::path ccJsonDirPath =
            Paths::getUTBotBuildDir(testGen.projectContext) / "stubs_build_files";
    auto stubFiles = Paths::findFilesInFolder(Paths::getStubsDirPath(testGen.projectContext));
    stubFiles = Synchronizer::dropHeaders(stubFiles);
    CollectionUtils::erase_if(stubFiles, [this](fs::path const &stubPath) {
        fs::path sourcePath = Paths::stubPathToSourcePath(testGen.projectContext, stubPath);
        return CollectionUtils::contains(testGen.targetSources, sourcePath);
    });
    if (stubFiles.empty()) {
        return {};
    }
    printer::CCJsonPrinter::createDummyBuildDB(stubFiles, ccJsonDirPath);
    auto stubsCdb = CompilationUtils::getCompilationDatabase(ccJsonDirPath);
    tests::TestsMap stubFilesMap;
    for (const auto &file : stubFiles) {
        stubFilesMap[file].sourceFilePath = file;
    }
    Fetcher::Options::Value options = Fetcher::Options::Value::RETURN_TYPE_NAMES_ONLY;
    Fetcher fetcher(options, stubsCdb, stubFilesMap, nullptr, nullptr, ccJsonDirPath, true);
    fetcher.fetchWithProgress(testGen.progressWriter, "Finding stub files", true);
    CollectionUtils::FileSet stubFilesSet;
    auto signatureNamesSet = CollectionUtils::transformTo<std::unordered_set<std::string>>(
            signatures,
            [&](const tests::Tests::MethodDescription &signature) { return signature.name; });
    for (const auto &[filePath, stub]: stubFilesMap) {
        for (const auto &[methodName, methodDescription]: stub.methods) {
            if (CollectionUtils::contains(signatureNamesSet, methodName)) {
                stubFilesSet.insert(filePath);
                auto stubInfo = std::make_shared<types::FunctionInfo>(methodDescription.toFunctionInfo());
                for (auto &[_, tests]: testGen.tests) {
                    for (auto &[_, method]: tests.methods) {
                        method.stubsStorage->registerStub("", stubInfo, ((fs::path) filePath).replace_extension(".h"));
                    }
                }
            }
        }
    }
    return stubFilesSet;
}


tests::Tests StubGen::mergeSourceFileIntoStub(const tests::Tests &stubFile,
                                              const tests::Tests &srcFile) {
    tests::Tests::MethodsMap stubMethodsMap = stubFile.methods;
    tests::Tests::MethodsMap const &srcMethodsMap = srcFile.methods;

    for (const auto &[name, method] : srcMethodsMap) {
        if (!stubMethodsMap.contains(name) || !cmpMethodsDecl(stubMethodsMap[name], method)) {
            stubMethodsMap[name] = method;
            stubMethodsMap[name].sourceBody = std::nullopt;
        }
    }
    CollectionUtils::erase_if(
        stubMethodsMap, [&srcMethodsMap](const tests::Tests::MethodDescription &methodDescription) {
            return methodDescription.modifiers.isStatic ||
                   !srcMethodsMap.contains(methodDescription.name);
        });
    tests::Tests mergedFile = stubFile;
    mergedFile.sourceFilePath = srcFile.sourceFilePath;
    mergedFile.relativeFileDir = srcFile.relativeFileDir;
    mergedFile.methods = stubMethodsMap;
    mergedFile.mainHeader = srcFile.mainHeader;
    mergedFile.headersBeforeMainHeader = srcFile.headersBeforeMainHeader;
    return mergedFile;
}

bool StubGen::cmpMethodsDecl(const Tests::MethodDescription &decl1,
                             const tests::Tests::MethodDescription &decl2) {
    if (decl1.name != decl2.name || decl1.returnType.typeName() != decl2.returnType.typeName() ||
        decl1.params.size() != decl2.params.size()) {
        return false;
    }
    for (size_t i = 0; i < decl1.params.size(); i++) {
        if (decl1.params[i].type.typeName() != decl2.params[i].type.typeName() ||
            decl1.params[i].name != decl2.params[i].name) {
            return false;
        }
    }
    return true;
}

Result<CollectionUtils::FileSet> StubGen::getStubSetForObject(const fs::path &objectFilePath) {
    ShellExecTask::ExecutionParameters nmCommand(
            Paths::getLLVMnm(),
            { "--print-file-name", "--undefined-only", "--just-symbol-name", objectFilePath });
    auto [out, status, _] = ShellExecTask::runShellCommandTask(nmCommand, testGen.serverBuildDir);
    if (status != 0) {
        std::string errorMessage =
                StringUtils::stringFormat("llvm-nm on %s failed: %s", objectFilePath, out);
        LOG_S(ERROR) << errorMessage;
        return errorMessage;
    }
    auto symbols =
            CollectionUtils::transform(StringUtils::split(out, '\n'), [](std::string const &line) {
                return StringUtils::splitByWhitespaces(line).back();
            });
    CollectionUtils::erase_if(symbols, [](std::string const &symbol) {
        return StringUtils::startsWith(symbol, "__ubsan") ||
               StringUtils::startsWith(symbol, "klee_");
    });
    auto signatures = CollectionUtils::transform(symbols, [](std::string const &symbol) {
        Tests::MethodDescription methodDescription;
        methodDescription.name = symbol;
        return methodDescription;
    });
    return findStubFilesBySignatures(signatures);
}
