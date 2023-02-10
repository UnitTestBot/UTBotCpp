#include "Synchronizer.h"

#include "clang-utils/SourceToHeaderRewriter.h"
#include "exceptions/FileSystemException.h"
#include "fetchers/Fetcher.h"
#include "printers/CCJsonPrinter.h"
#include "printers/SourceWrapperPrinter.h"
#include "printers/StubsPrinter.h"
#include "streams/stubs/StubsWriter.h"
#include "testgens/SnippetTestGen.h"
#include "utils/TypeUtils.h"

#include "loguru.h"

#include <iterator>
#include <utility>
#include <fstream>

using StubSet = std::unordered_set<StubOperator, HashUtils::StubHash>;

StubOperator::StubOperator(fs::path filePath, bool header) : sourceFilePath(std::move(filePath)), header(header) {}

StubOperator::StubOperator(fs::path filePath) : sourceFilePath(std::move(filePath)), header(Paths::isHeaderFile(this->sourceFilePath)) {}

bool StubOperator::operator==(const StubOperator &other) const {
    return this->sourceFilePath == other.sourceFilePath && this->header == other.header;
}

fs::path StubOperator::getStubPath(const utbot::ProjectContext &projectContext) const {
    return header ? Paths::sourcePathToStubHeaderPath(projectContext, sourceFilePath)
                    : Paths::sourcePathToStubPath(projectContext, sourceFilePath);
}

fs::path StubOperator::getSourceFilePath() const {
    return sourceFilePath;
}

bool StubOperator::isHeader() const {
    return header;
}

Synchronizer::Synchronizer(BaseTestGen *testGen,
                           types::TypesHandler::SizeContext *sizeContext)
    : testGen(testGen), sizeContext(sizeContext) {
}

long long Synchronizer::getFileOutdatedTime(const fs::path &filePath) const {
    return TimeUtils::convertFileToSystemClock(fs::last_write_time(filePath))
                           .time_since_epoch()
                           .count();
}

bool Synchronizer::isProbablyOutdatedStubs(const fs::path &srcFilePath) const {
    fs::path stubFilePath = Paths::sourcePathToStubPath(testGen->projectContext, srcFilePath);
    if (!fs::exists(stubFilePath)) {
        return true;
    }
    std::ifstream stubFile(stubFilePath);
    std::string sLine;
    getline(stubFile, sLine);
    long long stubTimestamp, srcTimestamp;
    try {
        stubTimestamp = std::stoll(sLine.substr(2));
    } catch (...) {
        return true;
    }
    if (!fs::exists(srcFilePath)) {
        srcTimestamp = time(NULL);
    } else {
        srcTimestamp = Synchronizer::getFileOutdatedTime(srcFilePath);
    }
    return stubTimestamp <= srcTimestamp;
}

bool Synchronizer::isProbablyOutdatedWrappers(const fs::path &srcFilePath) const {
    fs::path wrapperFilePath = Paths::getWrapperFilePath(testGen->projectContext, srcFilePath);
    if (!fs::exists(wrapperFilePath)) {
        return true;
    }
    long long wrapperTimestamp, srcTimestamp;
    wrapperTimestamp = Synchronizer::getFileOutdatedTime(wrapperFilePath);
    if (!fs::exists(srcFilePath)) {
        srcTimestamp = time(NULL);
    } else {
        srcTimestamp = Synchronizer::getFileOutdatedTime(srcFilePath);
    }
    return wrapperTimestamp <= srcTimestamp;
}

CollectionUtils::FileSet Synchronizer::getOutdatedSourcePaths() const {
    auto allFiles = getTargetSourceFiles();
    auto outdatedSources = CollectionUtils::filterOut(getTargetSourceFiles(), [this](fs::path const &sourcePath) {
        return !isProbablyOutdatedWrappers(sourcePath);
    });
    return outdatedSources;
}

StubSet Synchronizer::getOutdatedStubs() const {
    auto allFiles = getStubsFiles();
    auto outdatedStubs = CollectionUtils::filterOut(allFiles, [this](StubOperator const &stubOperator) {
        return !isProbablyOutdatedStubs(stubOperator.getSourceFilePath());
    });
    return outdatedStubs;
}

bool Synchronizer::removeStubIfSourceAbsent(const StubOperator &stub) const {
    if (!fs::exists(stub.getSourceFilePath())) {
        try {
            fs::remove(stub.getStubPath(testGen->projectContext));
            return true;
        } catch (const fs::filesystem_error &e) {
            std::string message = StringUtils::stringFormat("Failed to delete stub file '%s'", stub.getStubPath(testGen->projectContext));
            throw FileSystemException(message, e);
        }
        return false;
    }
    return false;
}

StubSet Synchronizer::getStubSetFromSources(const CollectionUtils::FileSet &sourcePaths) {
    StubSet result;
    for (bool header : {false, true}) {
        auto stubs = CollectionUtils::transformTo<StubSet>(
        sourcePaths, [header](const fs::path &sourcePath) {
            return StubOperator(sourcePath, header);
        });
        result = CollectionUtils::unionSet(stubs, result);
    }
    return result;
}

StubSet Synchronizer::dropHeaders(const StubSet &stubs) {
    return CollectionUtils::filterOut(stubs, [](const StubOperator &stub) {
        return stub.isHeader();
    });
}

CollectionUtils::FileSet Synchronizer::dropHeaders(const CollectionUtils::FileSet &files) {
    return CollectionUtils::filterOut(files, [](const fs::path &file) {
        return Paths::isHeaderFile(file);
    });
}

void Synchronizer::synchronize(const types::TypesHandler &typesHandler) {
    if (TypeUtils::isSameType<SnippetTestGen>(*this->testGen)) {
        return;
    }
    if (testGen->settingsContext.useStubs) {
        auto outdatedStubs = getOutdatedStubs();
        synchronizeStubs(outdatedStubs, typesHandler);
    }
    auto outdatedSourcePaths = getOutdatedSourcePaths();
    synchronizeWrappers(outdatedSourcePaths);
}

void Synchronizer::synchronizeStubs(StubSet &outdatedStubs,
                                    const types::TypesHandler &typesHandler) {
    StubSet allStubs = getStubsFiles();
    auto stubDirPath = Paths::getStubsDirPath(testGen->projectContext);
    prepareDirectory(stubDirPath);
    auto filesInFolder = Paths::findFilesInFolder(stubDirPath);
    auto presentStubs = CollectionUtils::transformTo<StubSet>(
    dropHeaders(filesInFolder), [this](const fs::path &stubPath) {
        return StubOperator(Paths::stubPathToSourcePath(this->testGen->projectContext, stubPath),
                            Paths::isHeaderFile(stubPath));
    });
    StubSet stubFiles = CollectionUtils::unionSet(allStubs, presentStubs);
    tests::TestsMap stubFilesMap, sourceFilesMap;
    for (const auto &outdatedStub : outdatedStubs) {
        removeStubIfSourceAbsent(outdatedStub);
        fs::path stubPath = outdatedStub.getStubPath(testGen->projectContext);
        if (fs::exists(stubPath)) {
            stubFilesMap[stubPath].sourceFilePath = stubPath;
        }
        sourceFilesMap[outdatedStub.getSourceFilePath()].sourceFilePath = outdatedStub.getSourceFilePath();
    }

    auto options = Fetcher::Options::Value::FUNCTION | Fetcher::Options::Value::INCLUDE | Fetcher::Options::Value::TYPE;

    auto stubFetcher =
        Fetcher(options, testGen->getProjectBuildDatabase()->compilationDatabase, sourceFilesMap, &testGen->types,
                &sizeContext->maximumAlignment,
                testGen->compileCommandsJsonPath, false);

    stubFetcher.fetchWithProgress(testGen->progressWriter, "Finding source files required for stubs", true);

    fs::path ccJsonStubDirPath =
            Paths::getUTBotBuildDir(testGen->projectContext) / "stubs_build_files";
    // todo: is it needed?
    auto stubsCdb = createStubsCompilationDatabase(stubFiles, ccJsonStubDirPath);

    auto sourceToHeaderRewriter =
    SourceToHeaderRewriter(testGen->projectContext, testGen->getProjectBuildDatabase()->compilationDatabase,
                           stubFetcher.getStructsToDeclare(), testGen->serverBuildDir);

    for (const StubOperator &outdatedStub : outdatedStubs) {
        fs::path stubPath = outdatedStub.getStubPath(testGen->projectContext);
        Tests const &methodDescription = stubFilesMap[stubPath];
        if (outdatedStub.isHeader()) {
            std::string code = sourceToHeaderRewriter.generateStubHeader(outdatedStub.getSourceFilePath());
            testGen->synchronizedStubs.emplace_back(stubPath, code);
        } else {
            tests::Tests newStubFile = StubGen::mergeSourceFileIntoStub(
                methodDescription, sourceFilesMap.at(outdatedStub.getSourceFilePath()));
            printer::StubsPrinter stubsPrinter(Paths::getSourceLanguage(stubPath));
            Stubs stubFile =
                stubsPrinter.genStubFile(newStubFile, typesHandler, testGen->projectContext);
            testGen->synchronizedStubs.emplace_back(stubFile);
        }
    }
    StubsWriter::writeStubsFilesOnServer(testGen->synchronizedStubs, testGen->projectContext.testDirPath);
}

std::shared_ptr<CompilationDatabase>
Synchronizer::createStubsCompilationDatabase(StubSet &stubFiles,
                                             const fs::path &ccJsonStubDirPath) const {
    printer::CCJsonPrinter::createDummyBuildDB(
        CollectionUtils::transformTo<CollectionUtils::FileSet>(
            stubFiles, [](const StubOperator &stub) { return stub.getSourceFilePath(); }),
        ccJsonStubDirPath);
    return CompilationUtils::getCompilationDatabase(ccJsonStubDirPath);
}

void Synchronizer::synchronizeWrappers(const CollectionUtils::FileSet &outdatedSourcePaths) const {
    auto sourceFilesNeedToRegenerateWrappers = outdatedSourcePaths;
    for (fs::path const &sourceFilePath : getTargetSourceFiles()) {
        if (!CollectionUtils::contains(sourceFilesNeedToRegenerateWrappers, sourceFilePath)) {
            auto wrapperFilePath =
                Paths::getWrapperFilePath(testGen->projectContext, sourceFilePath);
            if (!fs::exists(wrapperFilePath)) {
                sourceFilesNeedToRegenerateWrappers.insert(sourceFilePath);
            }
        }
    }
    ExecUtils::doWorkWithProgress(
        sourceFilesNeedToRegenerateWrappers, testGen->progressWriter,
        "Generating wrappers", [this](fs::path const &sourceFilePath) {
            SourceToHeaderRewriter sourceToHeaderRewriter(testGen->projectContext,
                                                          testGen->getProjectBuildDatabase()->compilationDatabase, nullptr,
                                                          testGen->serverBuildDir);
            std::string wrapper = sourceToHeaderRewriter.generateWrapper(sourceFilePath);
            printer::SourceWrapperPrinter(Paths::getSourceLanguage(sourceFilePath)).print(testGen->projectContext, sourceFilePath, wrapper);
        });
}

const CollectionUtils::FileSet &Synchronizer::getTargetSourceFiles() const {
    return testGen->getTargetBuildDatabase()->compilationDatabase->getAllFiles();
}

const CollectionUtils::FileSet &Synchronizer::getProjectSourceFiles() const {
    return testGen->getProjectBuildDatabase()->compilationDatabase->getAllFiles();
}

StubSet Synchronizer::getStubsFiles() const {
    return getStubSetFromSources(testGen->getProjectBuildDatabase()->compilationDatabase->getAllFiles());
}

void Synchronizer::prepareDirectory(const fs::path &stubDirectory) {
    fs::create_directories(stubDirectory);
    for (const auto &entry : fs::recursive_directory_iterator(stubDirectory)) {
        if (entry.is_regular_file()) {
            fs::path stubPath = entry.path();
            if (!Paths::isHeaderFile(stubPath)) {
                fs::path sourcePath =
                    Paths::stubPathToSourcePath(testGen->projectContext, stubPath);
                if (!CollectionUtils::contains(getProjectSourceFiles(), sourcePath)) {
                    LOG_S(DEBUG) << "Found extra file in stub directory: " << stubPath
                                 << ". Removing it.";
                    fs::remove(stubPath);
                }
            }
        }
    }
}
