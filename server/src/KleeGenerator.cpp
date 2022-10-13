#include "KleeGenerator.h"

#include "environment/EnvironmentPaths.h"
#include "exceptions/ExecutionProcessException.h"
#include "exceptions/FileSystemException.h"
#include "printers/DefaultMakefilePrinter.h"
#include "printers/HeaderPrinter.h"
#include "tasks/ShellExecTask.h"
#include "utils/FileSystemUtils.h"
#include "utils/KleeUtils.h"
#include "utils/LogUtils.h"
#include "utils/MakefileUtils.h"
#include "utils/SanitizerUtils.h"

#include "loguru.h"

using namespace tests;

static const std::string GENERATION_COMPILE_MAKEFILE = "GenerationCompileMakefile.mk";
static const std::string GENERATION_KLEE_MAKEFILE = "GenerationKleeMakefile.mk";

KleeGenerator::KleeGenerator(BaseTestGen *testGen, types::TypesHandler &typesHandler,
                             PathSubstitution filePathsSubstitution)
        : testGen(testGen), typesHandler(typesHandler),
          pathSubstitution(std::move(filePathsSubstitution)) {
    try {
        fs::create_directories(this->testGen->serverBuildDir);
        fs::create_directories(Paths::getLogDir(this->testGen->projectContext.projectName));
    } catch (const fs::filesystem_error &e) {
        throw FileSystemException("create_directories failed", e);
    }
}

std::vector<KleeGenerator::BuildFileInfo>
KleeGenerator::buildByCDb(const CollectionUtils::MapFileTo<fs::path> &filesToBuild,
                          const CollectionUtils::FileSet &stubSources) {
    LOG_SCOPE_FUNCTION(DEBUG);
    auto compileCommands = getCompileCommandsForKlee(filesToBuild, stubSources);
    printer::DefaultMakefilePrinter makefilePrinter;

    std::vector<fs::path> outfilePaths;
    for (const auto &compileCommand: compileCommands) {
        fs::path output = compileCommand.getOutput();
        outfilePaths.emplace_back(output);
        utbot::CompileCommand compileCommandWithChangingDirectory{compileCommand, true};
        makefilePrinter.declareTarget(output, {compileCommandWithChangingDirectory.getSourcePath()},
                                      {compileCommandWithChangingDirectory.toStringWithChangingDirectory()});
    }

    makefilePrinter.declareTarget(printer::DefaultMakefilePrinter::TARGET_ALL, outfilePaths, {});
    const fs::path makefile = testGen->serverBuildDir / GENERATION_COMPILE_MAKEFILE;
    FileSystemUtils::writeToFile(makefile, makefilePrinter.ss.str());

    auto command = MakefileUtils::MakefileCommand(testGen->projectContext, makefile,
                                                  printer::DefaultMakefilePrinter::TARGET_ALL);
    ExecUtils::ExecutionResult res = command.run();
    if (res.status != 0) {
        LOG_S(ERROR) << StringUtils::stringFormat("Make for \"%s\" failed.\nCommand: \"%s\"\n%s\n",
                                                  makefile, command.getFailedCommand(), res.output);
        throw ExecutionProcessException(
                command.getFailedCommand(),
                res.outPath.value()
        );
    }

    auto outFiles = CollectionUtils::transform(
            compileCommands, [](utbot::CompileCommand const &compileCommand) {
                return BuildFileInfo{compileCommand.getOutput(), compileCommand.getSourcePath()};
            });
    return outFiles;
}

std::vector<KleeGenerator::BuildFileInfo>
KleeGenerator::buildByCDb(const CollectionUtils::FileSet &filesToBuild,
                          const CollectionUtils::FileSet &stubSources) {
    CollectionUtils::MapFileTo<fs::path> filesMap;
    for (fs::path const &file: filesToBuild) {
        filesMap[file] = testGen->getTargetBuildDatabase()->getBitcodeFile(file);
    }
    return buildByCDb(filesMap, stubSources);
}

static std::string getUTBotClangCompilerPath(fs::path clientCompilerPath) {
    auto compilerName = CompilationUtils::getCompilerName(clientCompilerPath);
    switch (compilerName) {
        case CompilationUtils::CompilerName::GCC:
            return Paths::getUTBotClang();
        case CompilationUtils::CompilerName::GXX:
            return Paths::getUTBotClangPP();
        case CompilationUtils::CompilerName::CLANG:
            return Paths::getUTBotClang();
        case CompilationUtils::CompilerName::CLANGXX:
            return Paths::getUTBotClangPP();
        default:
            return clientCompilerPath;
    }
}

static const std::unordered_set<std::string> UNSUPPORTED_FLAGS_AND_OPTIONS_KLEE = {
        "--coverage",
        "-fbranch-target-load-optimize",
        "-fcx-fortran-rules",
        "-fipa-cp-clone",
        "-fipa-cp-cloneclang-10",
        "-fira-loop-pressure",
        "-fno-forward-propagate",
        "-fno-if-conversion",
        "-fno-sched-interblock",
        "-fno-sched-spec-insn-heuristic",
        "-fno-tree-dominator-opts",
        "-fno-tree-sink",
        "-fno-tree-sinkclang-10",
        "-fpredictive-commoning",
        "-fprofile-dir",
        "-freschedule-modulo-scheduled-loops",
        "-fsched2-use-superblocks",
        "-fsel-sched-reschedule-pipelined",
        "-ftree-loop-distribute-patterns",
};

std::optional<utbot::CompileCommand>
KleeGenerator::getCompileCommandForKlee(const fs::path &hintPath,
                                        const CollectionUtils::FileSet &stubSources,
                                        const std::vector<std::string> &flags,
                                        bool forStub) const {
    auto compilationUnitInfo = testGen->getClientCompilationUnitInfo(hintPath, forStub);
    auto command = compilationUnitInfo->command;
    auto srcFilePath = compilationUnitInfo->getSourcePath();
    std::string newCompilerPath = getUTBotClangCompilerPath(command.getBuildTool());
    command.setBuildTool(newCompilerPath);

    srcFilePath = pathSubstitution.substituteLineFlag(srcFilePath);
    if (CollectionUtils::contains(stubSources, srcFilePath)) {
        srcFilePath = Paths::sourcePathToStubPath(testGen->projectContext, srcFilePath);
    }
    command.setSourcePath(srcFilePath);

    auto outFilePath = (forStub ? testGen->getProjectBuildDatabase()->getBitcodeFile(compilationUnitInfo->getOutputFile())
                                : testGen->getTargetBuildDatabase()->getBitcodeFile(compilationUnitInfo->getOutputFile()));
    fs::create_directories(outFilePath.parent_path());
    command.setOutput(outFilePath);
    command.setOptimizationLevel("-O0");
    command.removeCompilerFlagsAndOptions(UNSUPPORTED_FLAGS_AND_OPTIONS_KLEE);
    std::vector<std::string> extraFlags{"-emit-llvm",
                                        "-c",
                                        "-Xclang",
                                        "-disable-O0-optnone",
                                        "-g",
                                        "-fstandalone-debug",
                                        "-fno-discard-value-names",
                                        "-fno-elide-constructors",
                                        "-D" + PrinterUtils::KLEE_MODE + "=1",
                                        SanitizerUtils::CLANG_SANITIZER_CHECKS_FLAG};
    if (Paths::isCXXFile(srcFilePath)) {
        command.addFlagToBegin(CompilationUtils::getIncludePath(Paths::getAccessPrivateLibPath()));
    }
    command.addFlagsToBegin(flags);
    command.addFlagsToBegin(extraFlags);
    command.addFlagToBegin(
            StringUtils::stringFormat("-iquote%s", compilationUnitInfo->getSourcePath().parent_path()));
    LOG_S(MAX) << "New compile command with klee required flags: " << command.toString();
    return command;
}

std::vector<utbot::CompileCommand>
KleeGenerator::getCompileCommandsForKlee(const CollectionUtils::MapFileTo<fs::path> &filesToBuild,
                                         const CollectionUtils::FileSet &stubSources) const {
    std::vector<utbot::CompileCommand> compileCommands;
    compileCommands.reserve(filesToBuild.size());
    for (const auto &[fileToBuild, bitcode]: filesToBuild) {
        auto optionalCommand = getCompileCommandForKlee(fileToBuild, stubSources, {}, false);
        if (optionalCommand.has_value()) {
            auto command = std::move(optionalCommand).value();
            command.setOutput(bitcode);
            compileCommands.emplace_back(command);
        }
    }
    return compileCommands;
}


Result<fs::path> KleeGenerator::defaultBuild(const fs::path &hintPath,
                                             const fs::path &sourceFilePath,
                                             const fs::path &buildDirPath,
                                             const std::vector<std::string> &flags) {
    LOG_SCOPE_FUNCTION(DEBUG);
    auto bitcodeFilePath = testGen->getTargetBuildDatabase()->getBitcodeFile(sourceFilePath);
    auto optionalCommand = getCompileCommandForKlee(hintPath, {}, flags, false);
    if (!optionalCommand.has_value()) {
        std::string message = StringUtils::stringFormat(
                "Couldn't get command for klee file: %s\n"
                "Please check if directory is in source directories in UTBot extension settings: %s",
                sourceFilePath, hintPath.parent_path().string());
        throw BaseException(std::move(message));
    }
    auto &command = optionalCommand.value();
    command.setSourcePath(sourceFilePath);
    command.setOutput(bitcodeFilePath);

    printer::DefaultMakefilePrinter makefilePrinter;
    auto commandWithChangingDirectory = utbot::CompileCommand(command, true);
    makefilePrinter.declareTarget(printer::DefaultMakefilePrinter::TARGET_BUILD,
                                  {commandWithChangingDirectory.getSourcePath()},
                                  {commandWithChangingDirectory.toStringWithChangingDirectory()});
    fs::path makefile = testGen->serverBuildDir / GENERATION_KLEE_MAKEFILE;
    FileSystemUtils::writeToFile(makefile, makefilePrinter.ss.str());

    auto makefileCommand = MakefileUtils::MakefileCommand(testGen->projectContext, makefile,
                                                          printer::DefaultMakefilePrinter::TARGET_BUILD);
    auto[out, status, _] = makefileCommand.run();
    if (status != 0) {
        LOG_S(ERROR) << "Compilation for " << sourceFilePath << " failed.\n"
                     << "Command: \"" << commandWithChangingDirectory.toString() << "\"\n"
                     << "Directory: " << buildDirPath << "\n"
                     << out << "\n";
        return out;
    }
    return command.getOutput();
}

Result<fs::path> KleeGenerator::defaultBuild(const fs::path &sourceFilePath,
                                             const fs::path &buildDirPath,
                                             const std::vector<std::string> &flags) {
    return defaultBuild(sourceFilePath, sourceFilePath, buildDirPath, flags);
}


fs::path KleeGenerator::writeKleeFile(
        printer::KleePrinter &kleePrinter,
        Tests const &tests,
        const std::shared_ptr<LineInfo> &lineInfo,
        const std::function<bool(tests::Tests::MethodDescription const &)> &methodFilter) {
    if (lineInfo) {
        return kleePrinter.writeTmpKleeFile(
                tests, testGen->serverBuildDir, pathSubstitution, lineInfo->predicateInfo, lineInfo->methodName,
                lineInfo->scopeName, lineInfo->forMethod, lineInfo->forClass, methodFilter);
    } else {
        return kleePrinter.writeTmpKleeFile(tests, testGen->serverBuildDir, pathSubstitution, std::nullopt,
                                            "", "", false, false, methodFilter);
    }
}

std::vector<fs::path> KleeGenerator::buildKleeFiles(const tests::TestsMap &testsMap,
                                                    const std::shared_ptr<LineInfo> &lineInfo) {
    std::vector<fs::path> outFiles;
    LOG_S(DEBUG) << "Building generated klee files...";
    printer::KleePrinter kleePrinter(&typesHandler, testGen->getTargetBuildDatabase(), utbot::Language::UNKNOWN);
    ExecUtils::doWorkWithProgress(
            testsMap, testGen->progressWriter, "Building generated klee files",
            [&](auto const &it) {
                const auto &[filename, tests] = it;
                if (lineInfo != nullptr && filename != lineInfo->filePath) {
                    return;
                }
                kleePrinter.srcLanguage = Paths::getSourceLanguage(filename);
                std::vector<std::string> includeFlags = {
                        CompilationUtils::getIncludePath(Paths::getFlagsDir(testGen->projectContext))};
                auto buildDirPath =
                        testGen->getClientCompilationUnitInfo(filename)->getDirectory();

                fs::path kleeFilePath = writeKleeFile(kleePrinter, tests, lineInfo);
                auto kleeFilesInfo =
                        testGen->getClientCompilationUnitInfo(
                                tests.sourceFilePath)->kleeFilesInfo;
                auto kleeBitcodeFile = defaultBuild(filename, kleeFilePath, buildDirPath, includeFlags);
                if (kleeBitcodeFile.isSuccess()) {
                    outFiles.emplace_back(kleeBitcodeFile.getOpt().value());
                    kleeFilesInfo->setAllAreCorrect(true);
                    LOG_S(MAX) << "Klee filepath: " << outFiles.back();
                } else {
                    if (lineInfo) {
                        throw BaseException("Couldn't compile klee file for current line.");
                    }
                    auto tempKleeFilePath = Paths::addSuffix(kleeFilePath, "_temp");
                    fs::copy(kleeFilePath, tempKleeFilePath, fs::copy_options::overwrite_existing);
                    LOG_S(DEBUG)
                    << "File " << kleeFilePath
                    << " couldn't be compiled so it's copy is backed up in " << tempKleeFilePath
                    << ". Proceeding with generating klee file containing restricted number "
                       "of functions";
                    std::unordered_set<std::string> correctMethods;
                    for (const auto &[methodName, methodDescription]: tests.methods) {
                        fs::path currentKleeFilePath = kleePrinter.writeTmpKleeFile(
                                tests, testGen->serverBuildDir, pathSubstitution, std::nullopt,
                                methodDescription.name,
                                methodDescription.getClassName(),
                                true, false);
                        auto currentKleeBitcodeFile =
                                defaultBuild(filename, currentKleeFilePath, buildDirPath, includeFlags);
                        if (currentKleeBitcodeFile.isSuccess()) {
                            correctMethods.insert(methodDescription.name);
                        } else {
                            std::stringstream message;
                            message << "Function '" << methodName
                                    << "' was skipped, as there was an error in compilation klee file "
                                       "for it";
                            LOG_S(WARNING) << message.str();
                            failedFunctions[filename].emplace_back(message.str());
                        }
                    }
                    kleeFilesInfo->setCorrectMethods(std::move(correctMethods));

                    auto kleeFilePath = writeKleeFile(
                            kleePrinter, tests, lineInfo,
                            [&kleeFilesInfo](tests::Tests::MethodDescription const &method) -> bool {
                                return kleeFilesInfo->isCorrectMethod(method.name);
                            });
                    auto kleeBitcodeFile =
                            defaultBuild(filename, kleeFilePath, buildDirPath, includeFlags);
                    if (kleeBitcodeFile.isSuccess()) {
                        outFiles.emplace_back(kleeBitcodeFile.getOpt().value());
                    } else {
                        throw BaseException("Couldn't compile klee file from correct methods.");
                    }
                }
            });
    return outFiles;
}

void KleeGenerator::parseKTestsToFinalCode(
    const utbot::ProjectContext &projectContext,
    tests::Tests &tests,
    const std::unordered_map<std::string, types::Type> &methodNameToReturnTypeMap,
    const std::vector<MethodKtests> &kleeOutput,
    const std::shared_ptr<LineInfo> &lineInfo,
    bool verbose) {
    for (const auto &batch : kleeOutput) {
        bool filterByFlag = (lineInfo != nullptr && !lineInfo->forMethod && !lineInfo->forClass &&
                             !lineInfo->predicateInfo.has_value());
        tests::KTestObjectParser KTestObjectParser(typesHandler);
        KTestObjectParser.parseKTest(batch, tests, methodNameToReturnTypeMap, filterByFlag,
                                     lineInfo);
    }
    printer::TestsPrinter testsPrinter(testGen->projectContext, &typesHandler,
                                       Paths::getSourceLanguage(tests.sourceFilePath));
    for (auto it = tests.methods.begin(); it != tests.methods.end(); it++) {
        const std::string &methodName = it.key();
        Tests::MethodDescription &methodDescription = it.value();
        if (lineInfo) {
            bool methodNotMatch = lineInfo->forMethod && methodName != lineInfo->methodName;
            bool classNotMatch = lineInfo->forClass && methodDescription.isClassMethod() &&
                                 methodDescription.getClassName().value() != lineInfo->scopeName;
            if (methodNotMatch || classNotMatch) {
                continue;
            }
        }
        if (methodDescription.testCases.empty()) {
            continue;
        }
        auto predicate =
            lineInfo ? lineInfo->predicateInfo : std::optional<LineInfo::PredicateInfo>{};

        testsPrinter.genCode(methodDescription, predicate, verbose);
    }

    printer::HeaderPrinter(Paths::getSourceLanguage(tests.sourceFilePath))
        .print(tests.testHeaderFilePath, tests.sourceFilePath, tests.headerCode);
    testsPrinter.joinToFinalCode(tests, tests.testHeaderFilePath);
    LOG_S(DEBUG) << "Generated code for " << tests.methods.size() << " tests";
}

fs::path KleeGenerator::getBitcodeFile(const fs::path &sourcePath) const {
    return testGen->getTargetBuildDatabase()->getBitcodeFile(sourcePath);
}

void KleeGenerator::handleFailedFunctions(tests::TestsMap &testsMap) {
    for (auto &[fileName, tests]: failedFunctions) {
        for (const auto &commentBlock: tests) {
            testsMap[fileName].commentBlocks.emplace_back(commentBlock);
        }
    }
}
