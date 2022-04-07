/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "StubGen.h"

#include "FeaturesFilter.h"
#include "Paths.h"
#include "StubSourcesFinder.h"
#include "Synchronizer.h"
#include "TimeExecStatistics.h"
#include "clang-utils/SourceToHeaderRewriter.h"
#include "printers/CCJsonPrinter.h"
#include "streams/stubs/StubsWriter.h"


StubGen::StubGen(BaseTestGen &testGen) : testGen(testGen) {
}

CollectionUtils::FileSet StubGen::getStubSources(const fs::path &target) {
    if (!testGen.needToBeMocked() || !testGen.settingsContext.useStubs) {
        return {};
    }
    fs::path testedFilePath = *testGen.testingMethodsSourcePaths.begin();
    auto stubSources = StubSourcesFinder(testGen.buildDatabase).excludeFind(testedFilePath, target);
    return { stubSources.begin(), stubSources.end() };
};

CollectionUtils::FileSet
StubGen::findStubFilesBySignatures(const vector<tests::Tests::MethodDescription> &signatures) {
    fs::path ccJsonDirPath =
        Paths::getTmpDir(testGen.projectContext.projectName) / "stubs_build_files";
    auto stubFiles =
        Paths::findFilesInFolder(Paths::getStubsDirPath(testGen.projectContext));
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
    Fetcher::Options::Value options = Fetcher::Options::Value::FUNCTION_NAMES_ONLY;
    Fetcher fetcher(options, stubsCdb, stubFilesMap, nullptr,
                              nullptr, nullptr, ccJsonDirPath, true);
    fetcher.fetchWithProgress(testGen.progressWriter, "Finding stub files", true);
    CollectionUtils::FileSet stubFilesSet;
    auto signatureNamesSet = CollectionUtils::transformTo<std::unordered_set<std::string>>(
        signatures,
        [&](const tests::Tests::MethodDescription &signature) { return signature.name; });
    for (const auto &[filePath, stub] : stubFilesMap) {
        for (const auto &[methodName, methodDescription] : stub.methods) {
            if (CollectionUtils::contains(signatureNamesSet, methodName)) {
                stubFilesSet.insert(filePath);
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
