/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "Linker.h"

#include "KleeGenerator.h"
#include "Paths.h"
#include "Synchronizer.h"
#include "RunCommand.h"
#include "environment/EnvironmentPaths.h"
#include "exceptions/ExecutionProcessException.h"
#include "exceptions/FileNotPresentedInCommandsException.h"
#include "exceptions/NoTestGeneratedException.h"
#include "printers/DefaultMakefilePrinter.h"
#include "printers/NativeMakefilePrinter.h"
#include "stubs/StubGen.h"
#include "testgens/FileTestGen.h"
#include "testgens/FolderTestGen.h"
#include "testgens/SnippetTestGen.h"
#include "utils/DynamicLibraryUtils.h"
#include "utils/FileSystemUtils.h"
#include "utils/LinkerUtils.h"
#include "utils/LogUtils.h"
#include "utils/MakefileUtils.h"
#include "utils/SanitizerUtils.h"
#include "utils/TypeUtils.h"
#include "utils/path/FileSystemPath.h"

#include <unordered_set>
#include <utility>
#include <exceptions/FileNotPresentedInArtifactException.h>

using std::string;
using std::vector;
using TypeUtils::isDerivedFrom;
using TypeUtils::isSameType;
std::vector<fs::path> sourcePaths;

bool Linker::isForOneFile() {
    return (lineInfo != nullptr) || isSameType<FileTestGen>(testGen) ||
           isSameType<SnippetTestGen>(testGen);
}

fs::path Linker::getSourceFilePath() {
    auto fileTestGen = dynamic_cast<FileTestGen *>(&testGen);
    auto snippetTestGen = dynamic_cast<SnippetTestGen *>(&testGen);
    fs::path sourcePath;
    if (lineInfo != nullptr) {
        return lineInfo->filePath;
    } else if (fileTestGen != nullptr) {
        return fileTestGen->filepath;
    } else if (snippetTestGen != nullptr) {
        return snippetTestGen->sourcePaths[0];
    } else {
        throw BaseException(
            "Couldn't handle test generation of current type in function getSourcePath");
    }
}

Result<Linker::LinkResult> Linker::linkForTarget(const fs::path &target, const fs::path &sourceFilePath,
                           const std::shared_ptr<const BuildDatabase::ObjectFileInfo> &compilationUnitInfo,
                           const fs::path &objectFile) {
    testGen.setTargetPath(target);

    auto siblings = testGen.buildDatabase->getArchiveObjectFiles(target);
    checkSiblingsExist(siblings);
    auto stubSources = stubGen.getStubSources(target);

    CollectionUtils::MapFileTo<fs::path> filesToLink;
    filesToLink.insert({ objectFile, compilationUnitInfo->kleeFilesInfo->getKleeBitcodeFile() });
    for (const auto &sibling : siblings) {
        if (sibling != objectFile) {
            auto siblingCompilationUnitInfo =
                testGen.buildDatabase->getClientCompilationUnitInfo(sibling);
            fs::path siblingObjectFile = siblingCompilationUnitInfo->getOutputFile();
            fs::path bitcodeFile = testGen.buildDatabase->getBitcodeForSource(
                siblingCompilationUnitInfo->getSourcePath());
            if (CollectionUtils::contains(stubSources,
                                          siblingCompilationUnitInfo->getSourcePath())) {
                bitcodeFile =
                    LinkerUtils::applySuffix(bitcodeFile, BuildResult::Type::ALL_STUBS, "");
            }
            filesToLink.emplace(siblingObjectFile, bitcodeFile);
        }
    }
    kleeGenerator->buildByCDb(filesToLink, stubSources);

    auto linkUnitInfo = testGen.buildDatabase->getClientLinkUnitInfo(sourceFilePath);
    std::optional<fs::path> moduleOutput = linkUnitInfo->getOutput();
    string suffixForParentOfStubs =
        StringUtils::stringFormat("___%s", Paths::mangle(moduleOutput.value().filename()));

    auto stubsSetResult = link(filesToLink, target, suffixForParentOfStubs, sourceFilePath, stubSources);
    return stubsSetResult;
}

std::vector<fs::path> Linker::getTargetList(const fs::path &sourceFile, const fs::path &objectFile) const {
    if (testGen.hasAutoTarget()) {
        return testGen.buildDatabase->autoTargetListForFile(sourceFile, objectFile);
    } else {
        return {testGen.getTargetPath()};
    }
}

void Linker::linkForOneFile(const fs::path &sourceFilePath) {
    ExecUtils::throwIfCancelled();

    auto compilationUnitInfo = testGen.buildDatabase->getClientCompilationUnitInfo(sourceFilePath);
    fs::path objectFile = compilationUnitInfo->getOutputFile();

    if (CollectionUtils::contains(testedFiles, sourceFilePath)) {
        return;
    }
    if (!testGen.buildDatabase->isFirstObjectFileForSource(objectFile)) {
        return;
    }
    std::vector <fs::path> targets = getTargetList(sourceFilePath, objectFile);
    LOG_S(DEBUG) << "Linking bitcode for file " << sourceFilePath.filename();
    for (size_t i = 0; i < targets.size(); i++) {
        const auto& target = targets[i];
        LOG_S(DEBUG) << "Trying target: " << target.filename();
        auto result = linkForTarget(target, sourceFilePath, compilationUnitInfo, objectFile);
        if (result.isSuccess()) {
            auto [targetBitcode, stubsSet, _] = result.getOpt().value();
            addToGenerated({ objectFile }, targetBitcode);
            auto&& targetUnitInfo = testGen.buildDatabase->getClientLinkUnitInfo(target);
            testGen.buildDatabase->assignStubFilesToLinkUnit(targetUnitInfo, stubsSet);
            return;
        } else {
            LOG_S(DEBUG) << "Linkage for target " << target.filename() << " failed: " << result.getError()->c_str();
            if (i + 1 == targets.size()) {
                addToGenerated({ objectFile }, {});
                fs::path possibleBitcodeFileName =
                    testGen.buildDatabase->getBitcodeFile(testGen.getTargetPath());
                brokenLinkFiles.insert(possibleBitcodeFileName);
            }
        }
    }
}

Result<Linker::LinkResult> Linker::linkWholeTarget(const fs::path &target) {
    auto requestTarget = testGen.getTargetPath();
    testGen.setTargetPath(target);

    auto targetUnitInfo = testGen.buildDatabase->getClientLinkUnitInfo(target);
    auto siblings = testGen.buildDatabase->getArchiveObjectFiles(target);

    checkSiblingsExist(siblings);
    auto stubSources = stubGen.getStubSources(target);

    CollectionUtils::MapFileTo<fs::path> filesToLink;
    CollectionUtils::FileSet siblingObjectsToBuild;
    for (const fs::path &objectFile : siblings) {
        auto objectInfo = testGen.buildDatabase->getClientCompilationUnitInfo(objectFile);
        if (CollectionUtils::contains(stubSources, objectInfo->getSourcePath())) {
            continue;
        }
        bool insideFolder = true;
        if (auto folderTestGen = dynamic_cast<FolderTestGen *>(&testGen)) {
            fs::path folderGen = folderTestGen->folderPath;
            if (!Paths::isSubPathOf(folderGen, objectInfo->getSourcePath())) {
                insideFolder = false;
            }
        }
        if (testGen.buildDatabase->isFirstObjectFileForSource(objectFile) &&
            !CollectionUtils::contains(testedFiles, objectInfo->getSourcePath()) &&
            insideFolder) {
            filesToLink.insert(
                { objectFile, objectInfo->kleeFilesInfo->getKleeBitcodeFile() });
        } else {
            siblingObjectsToBuild.insert(objectInfo->getOutputFile());
            fs::path bitcodeFile =
                testGen.buildDatabase->getBitcodeForSource(objectInfo->getSourcePath());
            filesToLink.emplace(objectFile, bitcodeFile);
        }
    }

    kleeGenerator->buildByCDb(siblingObjectsToBuild, stubSources);
    auto result = link(filesToLink, target, "", std::nullopt, stubSources, false);
    //this is done in order to restore testGen.target in case of UTBot: auto
    testGen.setTargetPath(requestTarget);
    return result;
}

void Linker::linkForProject() {
    CollectionUtils::FileSet triedTargets;
    ExecUtils::doWorkWithProgress(
        testGen.tests, testGen.progressWriter, "Compiling and linking source files",
        [&](auto const &it) {
            fs::path const &sourceFile = it.first;
            auto compilationUnitInfo = testGen.buildDatabase->getClientCompilationUnitInfo(sourceFile);
            fs::path objectFile = compilationUnitInfo->getOutputFile();
            if (!CollectionUtils::contains(testedFiles, sourceFile)) {
                auto objectInfo = testGen.buildDatabase->getClientCompilationUnitInfo(sourceFile);
                if (objectInfo->linkUnit.empty()) {
                    LOG_S(WARNING) << "No executable or library found for current source file in "
                                      "link_commands.json: "
                                   << sourceFile;
                    return;
                }
                std::vector <fs::path> targets = getTargetList(sourceFile, objectFile);
                bool success = false;
                for (const auto &target : targets) {
                    if (!CollectionUtils::contains(triedTargets, target)) {
                        triedTargets.insert(target);
                        LOG_S(DEBUG) << "Linking target: " << target.filename();
                        auto result = linkWholeTarget(target);
                        if (result.isSuccess()) {
                            success = true;
                            auto linkres = result.getOpt().value();
                            auto objectFiles = CollectionUtils::transformTo<CollectionUtils::FileSet>(
                                    linkres.presentedFiles, [&](const fs::path &sourceFile) {
                                        auto compilationUnitInfo = testGen.buildDatabase->getClientCompilationUnitInfo(
                                                sourceFile);
                                        return compilationUnitInfo->getOutputFile();
                                    });
                            addToGenerated(objectFiles, linkres.bitcodeOutput);
                            auto &&targetUnitInfo = testGen.buildDatabase->getClientLinkUnitInfo(target);
                            testGen.buildDatabase->assignStubFilesToLinkUnit(targetUnitInfo, linkres.stubsSet);
                            break;
                        } else {
                            std::stringstream ss;
                            ss << "Couldn't link target " << target.filename() << " for file " << sourceFile;
                            LOG_S(DEBUG) << ss.str();
                        }
                    }
                }
                if (!success) {
                    LOG_S(WARNING) << "Unable to link file " << sourceFile << " with any target, skipping it";
                }
            }
        });
}

void Linker::addToGenerated(const CollectionUtils::FileSet &objectFiles, const fs::path &output) {
    for (const auto &objectFile : objectFiles) {
        auto objectInfo = testGen.buildDatabase->getClientCompilationUnitInfo(objectFile);
        const fs::path &sourcePath = objectInfo->getSourcePath();
        if (testGen.buildDatabase->isFirstObjectFileForSource(objectFile) &&
            !CollectionUtils::contains(testedFiles, sourcePath)) {
            testedFiles.insert(sourcePath);
            bitcodeFileName[sourcePath] = output;
        }
    }
}

void Linker::checkSiblingsExist(const CollectionUtils::FileSet &archivedFiles) const {
    for (const auto &objectFile : archivedFiles) {
        auto objectInfo = testGen.buildDatabase->getClientCompilationUnitInfo(objectFile);
        const fs::path &sourcePath = objectInfo->getSourcePath();
        if (!CollectionUtils::contains(testGen.sourcePaths, sourcePath)) {
            throw CompilationDatabaseException(
                "Source file is not registered: " + sourcePath.string() +
                "\nPlease check if directory is in source directories in UTBot extension "
                "settings: " +
                sourcePath.parent_path().string());
        }
    }
}

void Linker::prepareArtifacts() {
    if (isForOneFile()) {
        fs::path sourceFilePath = getSourceFilePath();
        linkForOneFile(sourceFilePath);
    } else {
        linkForProject();
    }
}

void Linker::writeMakefiles() {
    for (auto const &[sourcePath, makefile] : linkMakefiles) {
        auto makefilePath =
            Paths::getMakefilePathFromSourceFilePath(testGen.projectContext, sourcePath);
        FileSystemUtils::writeToFile(makefilePath, makefile);
    }
}

vector<tests::TestMethod> Linker::getTestMethods() {
    LOG_S(DEBUG) << StringUtils::stringFormat(
        "Linkage statistics:\nAll files: %d\nNumber of files with broken linkage: %d",
        testGen.tests.size(), brokenLinkFiles.size());
    std::vector<tests::TestMethod> testMethods;
    bool isAnyOneLinked = false;
    if (lineInfo == nullptr) {
        for (auto &[fileName, tests] : testGen.tests) {
            if (!tests.isFilePresentedInCommands) {
                if (isForOneFile()) {
                    throw FileNotPresentedInCommandsException(fileName);
                } else {
                    LOG_S(WARNING) << FileNotPresentedInCommandsException::createMessage(fileName);
                }
                continue;
            }
            if (!CollectionUtils::contains(bitcodeFileName, fileName)) {
                LOG_S(DEBUG) << "Bitcode file for source is missing: " << fileName;
                continue;
            }
            fs::path bitcodePath = bitcodeFileName.at(fileName);
            if (CollectionUtils::contains(brokenLinkFiles, bitcodePath)) {
                LOG_S(ERROR) << "Couldn't link bitcode file for current source file: " << fileName;
                continue;
            }
            isAnyOneLinked = true;
            for (const auto &[methodName, _] : tests.methods) {
                auto compilationUnitInfo =
                    testGen.buildDatabase->getClientCompilationUnitInfo(fileName);
                if (compilationUnitInfo->kleeFilesInfo->isCorrectMethod(methodName)) {
                    testMethods.emplace_back(methodName, bitcodePath, fileName);
                }
            }
        }
    } else {
        [&] {
            for (auto &[fileName, tests] : testGen.tests) {
                if (CollectionUtils::contains(brokenLinkFiles, bitcodeFileName.at(fileName))) {
                    LOG_S(ERROR) << "Couldn't link bitcode file for current source file: "
                                 << fileName;
                    continue;
                }
                isAnyOneLinked = true;
                if (fileName != lineInfo->filePath) {
                    continue;
                }
                for (const auto &[methodName, method] : tests.methods) {
                    if (methodName == lineInfo->methodName ||
                        (lineInfo->forClass && method.className == lineInfo->scopeName)) {
                        auto compilationUnitInfo =
                            testGen.buildDatabase->getClientCompilationUnitInfo(fileName);
                        if (compilationUnitInfo->kleeFilesInfo->isCorrectMethod(methodName)) {
                            tests::TestMethod testMethod{ methodName,
                                                          bitcodeFileName.at(lineInfo->filePath),
                                                          fileName };
                            testMethods.emplace_back(testMethod);
                        }
                        if (!lineInfo->forClass)
                            return;
                    }
                }
            }
        }();
    }
    if (!isAnyOneLinked) {
        throw CompilationDatabaseException("Couldn't link any files");
    }
    if (testMethods.empty()) {
        throw NoTestGeneratedException("Couldn't generate tests for any method");
    }
    return testMethods;
}

Linker::Linker(BaseTestGen &testGen,
               StubGen stubGen,
               shared_ptr<LineInfo> lineInfo,
               shared_ptr<KleeGenerator> kleeGenerator)
    : testGen(testGen), stubGen(std::move(stubGen)), lineInfo(std::move(lineInfo)),
      kleeGenerator(kleeGenerator) {
}

Result<Linker::LinkResult> Linker::link(const CollectionUtils::MapFileTo<fs::path> &bitcodeFiles,
                                        const fs::path &target,
                                        std::string const &suffixForParentOfStubs,
                                        const std::optional<fs::path> &testedFilePath,
                                        const CollectionUtils::FileSet &stubSources,
                                        bool errorOnMissingBitcode) {
    LOG_SCOPE_FUNCTION(DEBUG);
    std::stringstream logStream;
    logStream << "Linking files";
    if (LogUtils::isMaxVerbosity()) {
        logStream << ":\n";
        for (const auto &[objectFile, bitcodeFile] : bitcodeFiles) {
            logStream << bitcodeFile << "\n";
        }
    }
    LOG_S(DEBUG) << logStream.str();
    for (const auto &[objectFile, bitcodeFile] : bitcodeFiles) {
        if (!fs::exists(bitcodeFile)) {
            throw CompilationDatabaseException("Trying to link file that doesn't exist: " +
                                               bitcodeFile.string());
        }
    }

    ExecUtils::throwIfCancelled();

    fs::path stubsMakefile = testGen.serverBuildDir / "GenerationStubsMakefile.mk";
    fs::remove(stubsMakefile);
    FileSystemUtils::writeToFile(stubsMakefile, "");

    printer::DefaultMakefilePrinter bitcodeLinkMakefilePrinter;
    printer::NativeMakefilePrinter nativeLinkMakefilePrinter{ testGen, &stubSources };
    bitcodeLinkMakefilePrinter.declareInclude(stubsMakefile);
    auto[targetBitcode, _] = addLinkTargetRecursively(target, bitcodeLinkMakefilePrinter, stubSources, bitcodeFiles,
                                                      suffixForParentOfStubs, false, testedFilePath);

    fs::path linkMakefile = testGen.serverBuildDir / "GenerationLinkMakefile.mk";
    FileSystemUtils::writeToFile(linkMakefile, bitcodeLinkMakefilePrinter.ss.str());

    auto command =
        MakefileUtils::makefileCommand(testGen.projectContext, linkMakefile, targetBitcode);
    auto [out, status, logFilePath] = command.run(testGen.serverBuildDir);
    if (status != 0) {
        string errorMessage =
            StringUtils::stringFormat("Make for \"%s\" failed.\nCommand: \"%s\"\n%s\n",
                                      linkMakefile, command.getFailedCommand(), out);
        LOG_S(ERROR) << errorMessage;
        return errorMessage;
    }
    CollectionUtils::FileSet stubsSet, presentedFiles;
    if (Paths::isLibraryFile(target)) {
        auto stubsSetResult = generateStubsMakefile(target, targetBitcode, stubsMakefile);
        if (!stubsSetResult.isSuccess()) {
            return stubsSetResult.getError().value();
        }
        stubsSet = stubsSetResult.getOpt().value();
        auto linkResult = linkWithStubsIfNeeded(linkMakefile, targetBitcode);
        if (!linkResult.isSuccess()) {
            return linkResult.getError().value();
        }
        nativeLinkMakefilePrinter.addStubs(stubsSet);
    }

    bool success = irParser.parseModule(targetBitcode, testGen.tests);
    if (!success) {
        string message = StringUtils::stringFormat("Couldn't parse module: ", targetBitcode);
        throw CompilationDatabaseException(message);
    }

    for (const auto& [sourceFile, test] : testGen.tests) {
        if (!test.isFilePresentedInArtifact) {
            if (errorOnMissingBitcode) {
                string message = FileNotPresentedInArtifactException::createMessage(sourceFile);
                return message;
            }
        } else {
            presentedFiles.insert(sourceFile);
        }
    }

    nativeLinkMakefilePrinter.addLinkTargetRecursively(target, suffixForParentOfStubs);
    for (auto const &[objectFile, _] : bitcodeFiles) {
        auto compilationUnitInfo = testGen.buildDatabase->getClientCompilationUnitInfo(objectFile);
        auto sourcePath = compilationUnitInfo->getSourcePath();
        if (CollectionUtils::containsKey(testGen.tests, sourcePath)) {
            printer::NativeMakefilePrinter makefilePrinter{ nativeLinkMakefilePrinter, sourcePath };
            linkMakefiles[sourcePath] = makefilePrinter.ss.str();
        }
    }
    return LinkResult{ targetBitcode, stubsSet, presentedFiles };
};

static const string STUB_BITCODE_FILES_NAME = "STUB_BITCODE_FILES";
static const string STUB_BITCODE_FILES = "$(STUB_BITCODE_FILES)";

Result<CollectionUtils::FileSet> Linker::generateStubsMakefile(
    const fs::path &root, const fs::path &outputFile, const fs::path &stubsMakefile) const {
    ShellExecTask::ExecutionParameters nmCommand(
        Paths::getLLVMnm(),
        { "--print-file-name", "--undefined-only", "--just-symbol-name", outputFile });
    auto [out, status, _] = ShellExecTask::runShellCommandTask(nmCommand, testGen.serverBuildDir);
    if (status != 0) {
        string errorMessage =
            StringUtils::stringFormat("llvm-nm on %s failed: %s", outputFile, out);
        LOG_S(ERROR) << errorMessage;
        return errorMessage;
    }
    auto symbols =
        CollectionUtils::transform(StringUtils::split(out, '\n'), [](string const &line) {
            return StringUtils::splitByWhitespaces(line).back();
        });
    CollectionUtils::erase_if(symbols, [](std::string const &symbol) {
        return StringUtils::startsWith(symbol, "__ubsan") ||
               StringUtils::startsWith(symbol, "klee_");
    });
    auto signatures = CollectionUtils::transform(symbols, [](string const &symbol) {
        Tests::MethodDescription methodDescription;
        methodDescription.name = symbol;
        return methodDescription;
    });
    auto rootLinkUnitInfo = testGen.buildDatabase->getClientLinkUnitInfo(root);
    auto stubsSet = StubGen(testGen).findStubFilesBySignatures(signatures);
    printer::DefaultMakefilePrinter makefilePrinter;
    auto bitcodeStubFiles = CollectionUtils::transformTo<std::vector<fs::path>>(
        Synchronizer::dropHeaders(stubsSet), [this, &makefilePrinter](const fs::path &stubPath) {
            fs::path sourcePath = Paths::stubPathToSourcePath(testGen.projectContext, stubPath);
            fs::path bitcodeFile = kleeGenerator->getBuildDatabase()->getBitcodeFile(sourcePath);
            bitcodeFile = Paths::getStubBitcodeFilePath(bitcodeFile);
            auto command = kleeGenerator->getCompileCommandForKlee(sourcePath, {}, {});
            command->setSourcePath(stubPath);
            command->setOutput(bitcodeFile);
            makefilePrinter.declareTarget(bitcodeFile, { stubPath },
                                          { command.value().toStringWithChangingDirectory() });
            return bitcodeFile;
        });
    makefilePrinter.declareVariable(STUB_BITCODE_FILES_NAME,
                                    StringUtils::joinWith(bitcodeStubFiles, " "));
    FileSystemUtils::writeToFile(stubsMakefile, makefilePrinter.ss.str());
    return stubsSet;
}

Result<utbot::Void> Linker::linkWithStubsIfNeeded(const fs::path &linkMakefile, const fs::path &targetBitcode) const {
    //We already have .bc file for target. If we don't remove this file, Makefile won't execute target "all",
    //because neither stub files, nor .bc change. However, current .bc file is incorrect, because it has compiled without stubs,
    //so it has external functions without body.
    bool removeStatus = fs::remove(targetBitcode);
    if (!removeStatus) {
        string errorMessage =
            StringUtils::stringFormat("Can't remove file: %s", targetBitcode);
        LOG_S(ERROR) << errorMessage;
        return errorMessage;
    }

    auto command = MakefileUtils::makefileCommand(testGen.projectContext, linkMakefile, "all");
    auto [out, status, _] = command.run(testGen.serverBuildDir);
    if (status != 0) {
        string errorMessage =
            StringUtils::stringFormat("link with stubs failed: %s", command.getFailedCommand());
        LOG_S(ERROR) << errorMessage;
        return errorMessage;
    }
    return utbot::Void{};
}

std::string getArchiveArgument(std::string const &argument,
                               fs::path const &workingDir,
                               CollectionUtils::MapFileTo<fs::path> const &dependencies,
                               BuildDatabase::TargetInfo const &linkUnitInfo,
                               fs::path const &output,
                               utbot::LinkCommand const &linkCommand,
                               bool &hasArchiveOption) {
    if (CollectionUtils::contains(linkUnitInfo.files, argument)) {
        fs::path bitcode = dependencies.at(argument);
        return fs::relative(bitcode, workingDir);
    }
    if (argument == linkUnitInfo.getOutput()) {
        return output;
    }
    if (argument == "-o") {
        return argument;
    }
    if (StringUtils::startsWith(argument, "-")) {
        return "";
    }
    hasArchiveOption |= !argument.empty() && argument != linkCommand.getLinker();
    return argument;
}

static void moveKleeTemporaryFileArgumentToBegin(std::vector<std::string> &arguments) {
    auto iteratorToCurrentFile = std::find_if(arguments.begin(), arguments.end(), [] (const std::string &argument) {
      return StringUtils::endsWith(argument, "_klee.bc");
    });
    if (iteratorToCurrentFile == arguments.end()) {
        LOG_S(ERROR) << "Don't find temporary klee file";
        return;
    }
    auto iteratorToSwap = std::find(arguments.begin(), arguments.end(), "-o");
    std::iter_swap(iteratorToSwap + 2, iteratorToCurrentFile);
}

static std::vector<utbot::LinkCommand>
getArchiveCommands(fs::path const &workingDir,
                   CollectionUtils::MapFileTo<fs::path> const &dependencies,
                   BuildDatabase::TargetInfo const &linkUnitInfo,
                   fs::path const &output) {
    auto commands = CollectionUtils::transform(
        linkUnitInfo.commands, [&](utbot::LinkCommand const &linkCommand) -> utbot::LinkCommand {
            bool hasArchiveOption = false;
            auto arguments = CollectionUtils::transformTo<std::vector<std::string>>(
                linkCommand.getCommandLine(), [&](std::string const &argument) -> std::string {
                    return getArchiveArgument(argument, workingDir, dependencies, linkUnitInfo,
                                              output, linkCommand, hasArchiveOption);
                });
            arguments.erase(arguments.begin());
            moveKleeTemporaryFileArgumentToBegin(arguments);

            if (hasArchiveOption) {
                arguments.insert(arguments.begin(), { "ar" });
            } else {
                arguments.insert(arguments.begin(), { "ar", "r" });
            }
            CollectionUtils::extend(arguments,
                                    std::vector<std::string>{ "--plugin", Paths::getLLVMgold() });
            utbot::LinkCommand result{ arguments, workingDir };
            result.setOutput(output);
            return result;
        });
    return commands;
}

static const std::vector<std::string> LD_GOLD_OPTIONS = {
    Paths::getLdGold(), "--plugin", Paths::getLLVMgold(),
    "-plugin-opt", "emit-llvm", "--allow-multiple-definition",
    "-relocatable"
};

static std::vector<std::string> getLinkActionsForRootLibrary(
    fs::path const &workingDir, std::vector<fs::path> const &dependencies, fs::path const &output) {
    std::vector<std::string> commandLine = LD_GOLD_OPTIONS;
    commandLine.emplace_back("--whole-archive");
    CollectionUtils::extend(
        commandLine,
        std::vector<std::string>{ StringUtils::joinWith(dependencies, " "), "-o", output });
    utbot::LinkCommand linkAction{ commandLine, workingDir };
    return { linkAction.toStringWithChangingDirectory() };
};

std::string Linker::getLinkArgument(std::string const &argument,
                                    fs::path const &workingDir,
                                    CollectionUtils::MapFileTo<fs::path> const &dependencies,
                                    BuildDatabase::TargetInfo const &linkUnitInfo,
                                    fs::path const &output) {
    if (CollectionUtils::contains(linkUnitInfo.files, argument)) {
        fs::path bitcode = dependencies.at(argument);
        fs::path relativePath = fs::relative(bitcode, workingDir);
        if (testGen.settingsContext.useStubs) {
            return StringUtils::stringFormat("--whole-archive %s --no-whole-archive", relativePath);
        } else {
            return relativePath;
        }
    }
    if (argument == linkUnitInfo.getOutput()) {
        return output;
    }
    if (argument == "-o") {
        return argument;
    }
    return "";
}

namespace {
    int getDependencyType(const std::string &dependency) {
        if (Paths::isObjectFile(dependency)) {
            return 1;
        }
        if (Paths::isStaticLibraryFile(dependency)) {
            return 2;
        }
        if (Paths::isSharedLibraryFile(dependency)) {
            return 3;
        }
        return 4;
    }

    const std::unordered_set<std::string> deprecatedFlags = {
        "--just-symbols",
        "-h",
        "-l",
        "--retain-symbols-file"
    };

    std::vector<std::string> sortLinkDependencies(const std::list<std::string> &linkCommandLine,
                                                  BuildDatabase::TargetInfo const &linkUnitInfo) {

        std::vector<string> commandLine;
        CollectionUtils::extend(commandLine, linkCommandLine);
        {
            std::string tmp;
            if (std::any_of(linkCommandLine.begin(), linkCommandLine.end(),
                            [&tmp](std::string const &argument) mutable {
                                bool res = CollectionUtils::contains(deprecatedFlags, argument);
                                if (res) {
                                    tmp = argument;
                                }
                                return res;
                            })) {
                LOG_S(ERROR) << "Link command line has deprecated argument: " << tmp;
            }
        }
        std::stable_partition(commandLine.begin(), commandLine.end(),
                              [&](std::string const &argument) {
                                  return !CollectionUtils::contains(linkUnitInfo.files, argument);
                              });
        auto dependencyIterator =
            std::find_if(commandLine.begin(), commandLine.end(), [&](std::string const &argument) {
                return CollectionUtils::contains(linkUnitInfo.files, argument);
            });
        std::sort(dependencyIterator, commandLine.end(),
                  [&](std::string const &arg1, std::string const &arg2) {
                      return getDependencyType(arg1) < getDependencyType(arg2);
                  });
        return commandLine;
    }
}

std::vector<utbot::LinkCommand>
Linker::getLinkActionsForExecutable(fs::path const &workingDir,
                                    CollectionUtils::MapFileTo<fs::path> const &dependencies,
                                    BuildDatabase::TargetInfo const &linkUnitInfo,
                                    fs::path const &output) {
    
    using namespace DynamicLibraryUtils;

    auto commands = CollectionUtils::transform(
        linkUnitInfo.commands, [&](utbot::LinkCommand const &linkCommand) -> utbot::LinkCommand {
            auto arguments = CollectionUtils::transformTo<std::vector<std::string>>(
                /*
                 * This is a workaround for ld.gold internal error.
                 * Dependencies are sorted in the way that the first ld.gold
                 * arguments are object files, then come libraries.
                 */
                sortLinkDependencies(linkCommand.getCommandLine(), linkUnitInfo),
                //-o fina qwkdwq lwqidjwq qwlidjwqli a.bc b.bc c.bc
                [&](std::string const &argument) -> std::string {
                    return getLinkArgument(argument, workingDir, dependencies, linkUnitInfo,
                                           output);
                });

            arguments.insert(arguments.begin(), LD_GOLD_OPTIONS.begin(), LD_GOLD_OPTIONS.end());
            utbot::LinkCommand result(arguments, workingDir);
            result.setOutput(output);
            return result;
        });
    return commands;
}

void Linker::declareRootLibraryTarget(printer::DefaultMakefilePrinter &bitcodeLinkMakefilePrinter,
                                      const fs::path &output,
                                      const vector<fs::path> &bitcodeDependencies,
                                      const fs::path &prefixPath,
                                      const utbot::RunCommand &removeAction,
                                      vector<utbot::LinkCommand> archiveActions) {
    fs::path temporaryOutput = Paths::addSuffix(output, "_tmp");
    utbot::RunCommand removeTemporaryAction =
        utbot::RunCommand::forceRemoveFile(temporaryOutput, testGen.serverBuildDir);
    std::vector<std::string> actions{ removeTemporaryAction.toStringWithChangingDirectory() };
    for (auto &archiveAction : archiveActions) {
        archiveAction.setOutput(temporaryOutput);
    }
    CollectionUtils::extend(
        actions, CollectionUtils::transform(
                     archiveActions, std::bind(&utbot::LinkCommand::toStringWithChangingDirectory,
                                               std::placeholders::_1)));
    bitcodeLinkMakefilePrinter.declareTarget(temporaryOutput, bitcodeDependencies, actions);

    auto linkActions =
        getLinkActionsForRootLibrary(prefixPath, { temporaryOutput, STUB_BITCODE_FILES }, output);
    linkActions.insert(linkActions.begin(), removeAction.toStringWithChangingDirectory());
    bitcodeLinkMakefilePrinter.declareTarget(output, { temporaryOutput, STUB_BITCODE_FILES },
                                             linkActions);
    bitcodeLinkMakefilePrinter.declareTarget("all", { output }, {});
}


BuildResult
Linker::addLinkTargetRecursively(const fs::path &fileToBuild,
                                 printer::DefaultMakefilePrinter &bitcodeLinkMakefilePrinter,
                                 const CollectionUtils::FileSet &stubSources,
                                 const CollectionUtils::MapFileTo<fs::path> &bitcodeFiles,
                                 std::string const &suffixForParentOfStubs,
                                 bool hasParent,
                                 const std::optional<fs::path> &testedFilePath) {
    using namespace std::placeholders;
    if (Paths::isObjectFile(fileToBuild)) {
        auto compilationUnitInfo = testGen.buildDatabase->getClientCompilationUnitInfo(fileToBuild);
        fs::path sourcePath = compilationUnitInfo->getSourcePath();
        BuildResult::Type type = CollectionUtils::contains(stubSources, sourcePath)
                                     ? BuildResult::Type::ALL_STUBS
                                     : BuildResult::Type::NO_STUBS;
        fs::path bitcode;
        if (compilationUnitInfo->getSourcePath() == testedFilePath) {
            bitcode = compilationUnitInfo->kleeFilesInfo->getKleeBitcodeFile();
        } else {
            bitcode = bitcodeFiles.at(fileToBuild);
        }
        return { bitcode, type };
    } else {
        auto linkUnit = testGen.buildDatabase->getClientLinkUnitInfo(fileToBuild);
        CollectionUtils::MapFileTo<fs::path> dependencies; // object file -> bitcode
        BuildResult::Type unitType = BuildResult::Type::NONE;
        for (auto const &subfile : linkUnit->files) {
            if (subfile != testedFilePath) {
                if (!CollectionUtils::containsKey(dependencies, subfile)) {
                    auto [dependency, childType] =
                        addLinkTargetRecursively(subfile, bitcodeLinkMakefilePrinter, stubSources, bitcodeFiles,
                                                 suffixForParentOfStubs, true, testedFilePath);
                    dependencies.emplace(subfile, std::move(dependency));
                    unitType |= childType;
                }
            }
        }
        auto bitcodeDependencies = CollectionUtils::getValues(dependencies);
        fs::path prefixPath = getPrefixPath(bitcodeDependencies, testGen.serverBuildDir);
        auto output = testGen.buildDatabase->getBitcodeFile(fileToBuild);
        output = LinkerUtils::applySuffix(output, unitType, suffixForParentOfStubs);
        if (Paths::isLibraryFile(fileToBuild)) {
            utbot::RunCommand removeAction =
                utbot::RunCommand::forceRemoveFile(output, testGen.serverBuildDir);
            auto archiveActions = getArchiveCommands(prefixPath, dependencies, *linkUnit, output);
            if (!hasParent) {
                declareRootLibraryTarget(bitcodeLinkMakefilePrinter, output, bitcodeDependencies,
                                         prefixPath, removeAction, archiveActions);

            } else {
                std::vector<std::string> actions = { removeAction.toStringWithChangingDirectory() };
                CollectionUtils::extend(
                    actions,
                    CollectionUtils::transform(
                        archiveActions,
                        std::bind(&utbot::LinkCommand::toStringWithChangingDirectory, _1)));
                bitcodeLinkMakefilePrinter.declareTarget(output, bitcodeDependencies, actions);
            }
        } else {
            auto linkActions =
                getLinkActionsForExecutable(prefixPath, dependencies, *linkUnit, output);
            auto actions = CollectionUtils::transform(
                linkActions, std::bind(&utbot::LinkCommand::toStringWithChangingDirectory, _1));
            bitcodeLinkMakefilePrinter.declareTarget(output, bitcodeDependencies, actions);
            bitcodeLinkMakefilePrinter.declareTarget("all", { output }, {});
        }
        return { output, unitType };
    }
}

fs::path Linker::getPrefixPath(const vector<fs::path> &dependencies, fs::path defaultPath) const {
    if (dependencies.empty()) {
        return defaultPath;
    }
    fs::path init = dependencies[0].parent_path();
    fs::path prefixPath = std::accumulate(dependencies.begin(), dependencies.end(), init,
                                          Paths::longestCommonPrefixPath);
    return std::max(prefixPath, defaultPath);
}
