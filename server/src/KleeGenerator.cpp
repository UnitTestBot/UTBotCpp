/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

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

KleeGenerator::KleeGenerator(
    utbot::ProjectContext projectContext,
    utbot::SettingsContext settingsContext,
    fs::path serverBuildDir,
    vector<fs::path> sourcesFilePaths,
    std::shared_ptr<clang::tooling::CompilationDatabase> compilationDatabase,
    types::TypesHandler &typesHandler,
    PathSubstitution filePathsSubstitution,
    std::shared_ptr<BuildDatabase> buildDatabase,
    ProgressWriter const *progressWriter)
    : projectContext(std::move(projectContext)),
      settingsContext(std::move(settingsContext)), projectTmpPath(std::move(serverBuildDir)),
      srcFiles(std::move(sourcesFilePaths)), compilationDatabase(std::move(compilationDatabase)),
      typesHandler(typesHandler), pathSubstitution(std::move(filePathsSubstitution)),
      buildDatabase(std::move(buildDatabase)), progressWriter(progressWriter) {
    try {
        fs::create_directories(this->projectTmpPath);
        fs::create_directories(Paths::getLogDir(this->projectContext.projectName));
    } catch (const fs::filesystem_error &e) {
        throw FileSystemException("create_directories failed", e);
    }
}

vector<KleeGenerator::BuildFileInfo>
KleeGenerator::buildByCDb(const CollectionUtils::MapFileTo<fs::path> &filesToBuild,
                          const CollectionUtils::FileSet &stubSources) {
    LOG_SCOPE_FUNCTION(DEBUG);
    auto compileCommands = getCompileCommandsForKlee(filesToBuild, stubSources);
    printer::DefaultMakefilePrinter makefilePrinter;

    vector<fs::path> outfilePaths;
    for (const auto &compileCommand : compileCommands) {
        fs::path output = compileCommand.getOutput();
        outfilePaths.emplace_back(output);
        makefilePrinter.declareTarget(output, { compileCommand.getSourcePath() },
                                      { compileCommand.toStringWithChangingDirectory() });
    }

    makefilePrinter.declareTarget("all", outfilePaths, {});
    fs::path makefile = projectTmpPath / "GenerationCompileMakefile.mk";
    FileSystemUtils::writeToFile(makefile, makefilePrinter.ss.str());

    auto command = MakefileUtils::makefileCommand(projectContext, makefile, "all");
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
            return BuildFileInfo{ compileCommand.getOutput(), compileCommand.getSourcePath() };
        });
    return outFiles;
}

vector<KleeGenerator::BuildFileInfo>
KleeGenerator::buildByCDb(const CollectionUtils::FileSet &filesToBuild,
                          const CollectionUtils::FileSet &stubSources) {
    CollectionUtils::MapFileTo<fs::path> filesMap;
    for (fs::path const &file : filesToBuild) {
        filesMap[file] = buildDatabase->getBitcodeFile(file);
    }
    return buildByCDb(filesMap, stubSources);
}

static string getUTBotClangCompilerPath(fs::path clientCompilerPath) {
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

std::optional<utbot::CompileCommand>
KleeGenerator::getCompileCommandForKlee(const fs::path &hintPath,
                                        const CollectionUtils::FileSet &stubSources,
                                        const vector<string> &flags) const {
    auto compilationUnitInfo = buildDatabase->getClientCompilationUnitInfo(hintPath);
    auto command = compilationUnitInfo->command;
    auto srcFilePath = compilationUnitInfo->getSourcePath();
    string newCompilerPath = getUTBotClangCompilerPath(command.getCompiler());
    command.setCompiler(newCompilerPath);

    srcFilePath = pathSubstitution.substituteLineFlag(srcFilePath);
    if (CollectionUtils::contains(stubSources, srcFilePath)) {
        srcFilePath = Paths::sourcePathToStubPath(projectContext, srcFilePath);
    }
    command.setSourcePath(srcFilePath);

    auto outFilePath = buildDatabase->getBitcodeFile(compilationUnitInfo->getOutputFile());
    fs::create_directories(outFilePath.parent_path());
    command.setOutput(outFilePath);
    command.setOptimizationLevel("-O0");
    command.removeGccFlags();
    vector<string> extraFlags{ "-emit-llvm",
                               "-c",
                               "-Xclang",
                               "-disable-O0-optnone",
                               "-g",
                               "-fstandalone-debug",
                               "-fno-discard-value-names",
                               "-fno-elide-constructors",
                               "-D" + PrinterUtils::KLEE_MODE + "=1",
                               SanitizerUtils::CLANG_SANITIZER_CHECKS_FLAG };
    if(Paths::isCXXFile(srcFilePath)) {
        command.addFlagToBegin(StringUtils::stringFormat("-I%s", Paths::getAccessPrivateLibPath()));
    }
    command.addFlagsToBegin(flags);
    command.addFlagsToBegin(extraFlags);
    command.addFlagToBegin(
        StringUtils::stringFormat("-I%s", compilationUnitInfo->getSourcePath().parent_path()));
    LOG_S(MAX) << "New compile command with klee required flags: " << command.toString();
    return command;
}

vector<utbot::CompileCommand>
KleeGenerator::getCompileCommandsForKlee(const CollectionUtils::MapFileTo<fs::path> &filesToBuild,
                                         const CollectionUtils::FileSet &stubSources) const {
    vector<utbot::CompileCommand> compileCommands;
    compileCommands.reserve(filesToBuild.size());
    for (const auto &[fileToBuild, bitcode] : filesToBuild) {
        auto optionalCommand = getCompileCommandForKlee(fileToBuild, stubSources, {});
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
                                             const vector<string> &flags) {
    LOG_SCOPE_FUNCTION(DEBUG);
    auto bitcodeFilePath = buildDatabase->getBitcodeFile(sourceFilePath);
    auto optionalCommand = getCompileCommandForKlee(hintPath, {}, flags);
    if (!optionalCommand.has_value()) {
        string message = StringUtils::stringFormat(
            "Couldn't get command for klee file: %s\n"
            "Please check if directory is in source directories in UTBot extension settings: %s",
            sourceFilePath, hintPath.parent_path().string());
        throw BaseException(std::move(message));
    }
    auto &command = optionalCommand.value();
    command.setSourcePath(sourceFilePath);
    command.setOutput(bitcodeFilePath);
    auto [out, status, _] = ShellExecTask::executeUtbotCommand(command, buildDirPath, projectContext.projectName);
    if (status != 0) {
        LOG_S(ERROR) << "Compilation for " << sourceFilePath << " failed.\n"
                     << "Command: \"" << command.toString() << "\"\n"
                     << "Directory: " << buildDirPath << "\n"
                     << out << "\n";
        return out;
    }
    return command.getOutput();
}

Result<fs::path> KleeGenerator::defaultBuild(const fs::path &sourceFilePath,
                                             const fs::path &buildDirPath,
                                             const vector<string> &flags) {
    return defaultBuild(sourceFilePath, sourceFilePath, buildDirPath, flags);
}


fs::path KleeGenerator::writeKleeFile(
    printer::KleePrinter &kleePrinter,
    Tests const &tests,
    const std::shared_ptr<LineInfo> &lineInfo,
    const std::function<bool(tests::Tests::MethodDescription const &)> &methodFilter) {
    if (lineInfo) {
        return kleePrinter.writeTmpKleeFile(
            tests, projectTmpPath, pathSubstitution, lineInfo->predicateInfo, lineInfo->methodName,
            lineInfo->scopeName, lineInfo->forMethod, lineInfo->forClass, lineInfo->isConstructor, methodFilter);
    } else {
        return kleePrinter.writeTmpKleeFile(tests, projectTmpPath, pathSubstitution, std::nullopt,
                                            "", "", false, false, false, methodFilter);
    }
}

vector<fs::path> KleeGenerator::buildKleeFiles(const tests::TestsMap &testsMap,
                                               const std::shared_ptr<LineInfo> &lineInfo) {
    vector<fs::path> outFiles;
    LOG_S(DEBUG) << "Building generated klee files...";
    printer::KleePrinter kleePrinter(&typesHandler, buildDatabase,  utbot::Language::UNKNOWN);
    ExecUtils::doWorkWithProgress(
        testsMap, progressWriter, "Building generated klee files",
        [&](auto const &it) {
            const auto &[filename, tests] = it;
            if (lineInfo != nullptr && filename != lineInfo->filePath) {
                return;
            }
            kleePrinter.srcLanguage = Paths::getSourceLanguage(filename);
            auto includeFlags = { StringUtils::stringFormat("-I%s",
                                                            Paths::getFlagsDir(projectContext)) };
            auto buildDirPath =
                buildDatabase->getClientCompilationUnitInfo(filename)->getDirectory();

            fs::path kleeFilePath = writeKleeFile(kleePrinter, tests, lineInfo);
            auto kleeFilesInfo =
                buildDatabase->getClientCompilationUnitInfo(tests.sourceFilePath)->kleeFilesInfo;
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
                std::unordered_set<string> correctMethods;
                for (const auto &[methodName, methodDescription] : tests.methods) {
                    fs::path currentKleeFilePath = kleePrinter.writeTmpKleeFile(
                        tests, projectTmpPath, pathSubstitution, std::nullopt,
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
    tests::Tests &tests,
    const std::unordered_map<string, types::Type> &methodNameToReturnTypeMap,
    const vector<MethodKtests> &kleeOutput,
    const std::shared_ptr<LineInfo> &lineInfo,
    bool verbose) {
    for (const auto &batch : kleeOutput) {
        bool filterByFlag = (lineInfo != nullptr && !lineInfo->forMethod && !lineInfo->forClass &&
                             !lineInfo->predicateInfo.has_value());
        tests::KTestObjectParser KTestObjectParser(typesHandler);
        KTestObjectParser.parseKTest(batch, tests, methodNameToReturnTypeMap, filterByFlag,
                                     lineInfo);
    }
    printer::TestsPrinter testsPrinter(&typesHandler, Paths::getSourceLanguage(tests.sourceFilePath));
    for (auto it = tests.methods.begin(); it != tests.methods.end(); it++) {
        const string &methodName = it.key();
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

    printer::HeaderPrinter(Paths::getSourceLanguage(tests.sourceFilePath)).print(tests.testHeaderFilePath, tests.sourceFilePath,
                                   tests.headerCode);
    testsPrinter.joinToFinalCode(tests, tests.testHeaderFilePath);
    LOG_S(DEBUG) << "Generated code for " << tests.methods.size() << " tests";
}

shared_ptr<BuildDatabase> KleeGenerator::getBuildDatabase() const {
    return buildDatabase;
}

void KleeGenerator::handleFailedFunctions(tests::TestsMap &testsMap) {
    for (auto &[fileName, tests] : failedFunctions) {
        for (const auto &commentBlock : tests) {
            testsMap[fileName].commentBlocks.emplace_back(commentBlock);
        }
    }
}
