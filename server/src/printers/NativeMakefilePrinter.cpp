/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "NativeMakefilePrinter.h"
#include "Synchronizer.h"

#include "building/RunCommand.h"
#include "environment/EnvironmentPaths.h"
#include "exceptions/UnImplementedException.h"
#include "utils/ArgumentsUtils.h"
#include "utils/CompilationUtils.h"
#include "utils/DynamicLibraryUtils.h"
#include "utils/LinkerUtils.h"
#include "utils/SanitizerUtils.h"
#include "utils/StringUtils.h"

namespace printer {
    using namespace DynamicLibraryUtils;
    using StringUtils::stringFormat;

    static const string STUB_OBJECT_FILES_NAME = "STUB_OBJECT_FILES";
    static const string STUB_OBJECT_FILES = "$(STUB_OBJECT_FILES)";

    static const std::string FPIC_FLAG = "-fPIC";
    static const std::vector<std::string> SANITIZER_NEEDED_FLAGS = {
        "-g", "-fno-omit-frame-pointer", "-fno-optimize-sibling-calls"
    };
    static const std::string STATIC_FLAG = "-static";
    static const std::string SHARED_FLAG = "-shared";
    static const std::string OPTIMIZATION_FLAG = "-O0";

    static const std::string FORCE = ".FORCE";


    static void eraseIfWlOnly(string &argument) {
        if (argument == "-Wl") {
            argument = "";
        }
    }
    static void removeLinkerFlag(string &argument, string const &flag) {
        auto options = StringUtils::split(argument, ',');
        size_t erased = CollectionUtils::erase_if(options, [&flag](string const &option) {
            return StringUtils::startsWith(option, flag);
        });
        if (erased == 0) {
            return;
        }
        argument = StringUtils::joinWith(options, ",");
        eraseIfWlOnly(argument);
    }

    static void removeScriptFlag(string &argument) {
        removeLinkerFlag(argument, "--version-script");
    }

    static void removeSonameFlag(string &argument) {
        auto options = StringUtils::split(argument, ',');
        bool isSonameNext = false;
        std::vector<std::string> result;
        for (string const &option : options) {
            if (option == "-soname") {
                isSonameNext = true;
                continue;
            }
            if (isSonameNext) {
                isSonameNext = false;
                continue;
            }
            result.push_back(option);
        }
        argument = StringUtils::joinWith(result, ",");
        eraseIfWlOnly(argument);
    }

    NativeMakefilePrinter::NativeMakefilePrinter(
        utbot::ProjectContext projectContext,
        shared_ptr<BuildDatabase> buildDatabase,
        fs::path const& rootPath,
        fs::path primaryCompiler,
        CollectionUtils::FileSet const *stubSources)
        : projectContext(std::move(projectContext)), buildDatabase(buildDatabase), rootPath(std::move(rootPath)),
          primaryCompiler(std::move(primaryCompiler)),
          primaryCxxCompiler(CompilationUtils::toCppCompiler(this->primaryCompiler)),
          primaryCompilerName(CompilationUtils::getCompilerName(this->primaryCompiler)),
          primaryCxxCompilerName(CompilationUtils::getCompilerName(primaryCxxCompiler)),
          cxxLinker(CompilationUtils::toCppLinker(primaryCxxCompiler)),

          pthreadFlag(CompilationUtils::getPthreadFlag(primaryCxxCompilerName)),
          coverageLinkFlags(StringUtils::joinWith(
              CompilationUtils::getCoverageLinkFlags(primaryCxxCompilerName), " ")),
          sanitizerLinkFlags(SanitizerUtils::getSanitizeLinkFlags(primaryCxxCompilerName)),

          buildDirectory(Paths::getBuildDir(projectContext)),
          dependencyDirectory(buildDirectory / "dependencies"),
          artifacts({ buildDirectory, dependencyDirectory }), stubSources(stubSources) {
        init();
    }

    void NativeMakefilePrinter::init() {
        declareAction(stringFormat("$(shell mkdir -p %s >/dev/null)", buildDirectory));
        declareAction(stringFormat("$(shell mkdir -p %s >/dev/null)", dependencyDirectory));
        declareTarget(FORCE, {}, {});

        comment("gtest");
        fs::path gtestBuildDirectory = buildDirectory / "googletest";

        fs::path defaultPath = "default.c";
        std::vector<string> defaultGtestCompileCommandLine{ primaryCxxCompiler, "-c", "-std=c++11",
                                                            FPIC_FLAG, defaultPath };
        utbot::CompileCommand defaultGtestCompileCommand{ defaultGtestCompileCommandLine,
                                                          buildDirectory, defaultPath };
        gtestAllTargets(defaultGtestCompileCommand, gtestBuildDirectory);
        gtestMainTargets(defaultGtestCompileCommand, gtestBuildDirectory);
        comment("/gtest");
    }

    NativeMakefilePrinter::NativeMakefilePrinter(const BaseTestGen &testGen,
                                                 CollectionUtils::FileSet const *stubSources)
        : NativeMakefilePrinter(
              testGen.projectContext,
              testGen.buildDatabase,
              testGen.getTargetPath(),
              CompilationUtils::getBundledCompilerPath(CompilationUtils::getCompilerName(CompilationUtils::detectBuildCompilerPath(testGen.compilationDatabase))),
              stubSources) {
    }

    fs::path NativeMakefilePrinter::getTemporaryDependencyFile(fs::path const &file) {
        fs::path relativePath = fs::relative(file, projectContext.projectPath);
        return dependencyDirectory / Paths::addExtension(relativePath, ".Td");
    }

    fs::path NativeMakefilePrinter::getDependencyFile(fs::path const &file) {
        fs::path relativePath = fs::relative(file, projectContext.projectPath);
        return dependencyDirectory / Paths::addExtension(relativePath, ".d");
    }

    void NativeMakefilePrinter::gtestAllTargets(const utbot::CompileCommand &defaultCompileCommand,
                                                const fs::path &gtestBuildDir) {
        fs::path gtestAllSourceFile = Paths::getGtestLibPath() / "googletest/src/gtest-all.cc";
        fs::path gtestAllObjectFile = gtestBuildDir / "gtest-all.cc.o";

        auto gtestCompilationArguments = defaultCompileCommand;
        gtestCompilationArguments.setSourcePath(gtestAllSourceFile);
        gtestCompilationArguments.setOutput(gtestAllObjectFile);
        gtestCompilationArguments.addFlagsToBegin(
            { stringFormat("-I%s", Paths::getGtestLibPath() / "googletest/include"),
              stringFormat("-I%s", Paths::getGtestLibPath() / "googletest") });

        declareTarget(gtestAllObjectFile, { gtestAllSourceFile },
                      { gtestCompilationArguments.toStringWithChangingDirectory() });
        declareVariable("GTEST_ALL", gtestAllObjectFile);

        artifacts.push_back(gtestAllObjectFile);
    }

    void NativeMakefilePrinter::gtestMainTargets(const utbot::CompileCommand &defaultCompileCommand,
                                                 const fs::path &gtestBuildDir) {
        fs::path gtestMainSourceFile = Paths::getGtestLibPath() / "googletest/src/gtest_main.cc";
        fs::path gtestMainObjectFile = gtestBuildDir / "gtest_main.cc.o";

        auto gtestCompilationArguments = defaultCompileCommand;
        gtestCompilationArguments.setSourcePath(gtestMainSourceFile);
        gtestCompilationArguments.setOutput(gtestMainObjectFile);
        declareTarget(gtestMainObjectFile, { gtestMainSourceFile },
                      { gtestCompilationArguments.toStringWithChangingDirectory() });
        declareVariable("GTEST_MAIN", gtestMainObjectFile);

        artifacts.push_back(gtestMainObjectFile);
    }

    void NativeMakefilePrinter::addCompileTarget(
        const fs::path &sourcePath,
        const fs::path &output,
        const BuildDatabase::ObjectFileInfo &compilationUnitInfo) {
        auto compileCommand = compilationUnitInfo.command;
        fs::path compiler = CompilationUtils::getBundledCompilerPath(
                CompilationUtils::getCompilerName(compileCommand.getCompiler()));
        fs::path cxxCompiler = CompilationUtils::toCppCompiler(compiler);
        auto compilerName = CompilationUtils::getCompilerName(compiler);
        compileCommand.setCompiler(compiler);
        compileCommand.setSourcePath(sourcePath);
        compileCommand.setOutput(output);

        compileCommand.setOptimizationLevel(OPTIMIZATION_FLAG);
        compileCommand.addEnvironmentVariable("C_INCLUDE_PATH", "$UTBOT_LAUNCH_INCLUDE_PATH");
        compileCommand.addFlagToBegin(FPIC_FLAG);
        compileCommand.addFlagsToBegin(SANITIZER_NEEDED_FLAGS);
        compileCommand.addFlagsToBegin(
            CompilationUtils::getCoverageCompileFlags(primaryCompilerName));
        compileCommand.addFlagsToBegin(SanitizerUtils::getSanitizeCompileFlags(compilerName));

        fs::path temporaryDependencyFile = getTemporaryDependencyFile(sourcePath);
        fs::path dependencyFile = getDependencyFile(sourcePath);
        compileCommand.addFlagToBegin(
            stringFormat("-MT $@ -MMD -MP -MF %s", temporaryDependencyFile));
        compileCommand.addFlagToBegin(
            stringFormat("-I%s", compilationUnitInfo.getSourcePath().parent_path()));

        string makingDependencyDirectory =
            stringFormat("mkdir -p %s", dependencyFile.parent_path());
        string postCompileAction =
            stringFormat("mv -f %s %s", temporaryDependencyFile, dependencyFile);
        declareTarget(output, { sourcePath, dependencyFile },
                      { makingDependencyDirectory, compileCommand.toStringWithChangingDirectory(),
                        postCompileAction });

        artifacts.push_back(output);
    }

    BuildResult NativeMakefilePrinter::addObjectFile(const fs::path &objectFile,
                                                     const std::string &suffixForParentOfStubs) {

        auto compilationUnitInfo = buildDatabase->getClientCompilationUnitInfo(objectFile);
        fs::path sourcePath = compilationUnitInfo->getSourcePath();

        fs::path pathToCompile;
        BuildResult buildResult;
        if (CollectionUtils::contains(*stubSources, sourcePath)) {
            pathToCompile = Paths::sourcePathToStubPath(projectContext, sourcePath);
            fs::path recompiledFile = Paths::getRecompiledFile(projectContext, pathToCompile);
            buildResult = { recompiledFile, BuildResult::Type::ALL_STUBS };
        } else {
            if (Paths::isCXXFile(sourcePath)) {
                pathToCompile = sourcePath;
            } else {
                pathToCompile = Paths::getWrapperFilePath(projectContext, sourcePath);
            }
            fs::path recompiledFile =
                Paths::getRecompiledFile(projectContext, compilationUnitInfo->getOutputFile());
            buildResult = { recompiledFile, BuildResult::Type::NO_STUBS };
        }
        addCompileTarget(pathToCompile, buildResult.output, *compilationUnitInfo);
        return buildResult;
    }

    void NativeMakefilePrinter::addTestTarget(const fs::path &sourcePath) {
        auto compilationUnitInfo = buildDatabase->getClientCompilationUnitInfo(sourcePath);
        auto testCompilationCommand = compilationUnitInfo->command;
        testCompilationCommand.setCompiler(primaryCxxCompiler);
        testCompilationCommand.setOptimizationLevel(OPTIMIZATION_FLAG);
        testCompilationCommand.filterCFlags();
        testCompilationCommand.removeIncludeFlags();
        testCompilationCommand.addFlagToBegin(FPIC_FLAG);
        testCompilationCommand.addFlagsToBegin(SANITIZER_NEEDED_FLAGS);

        fs::path testSourcePath = Paths::sourcePathToTestPath(projectContext, sourcePath);
        fs::path compilationDirectory = compilationUnitInfo->getDirectory();
        fs::path testObjectDir = Paths::getTestObjectDir(projectContext);
        fs::path testSourceRelativePath = fs::relative(testSourcePath, projectContext.testDirPath);
        fs::path testObjectPath = testObjectDir / Paths::addExtension(testSourceRelativePath, ".o");
        fs::path testObjectRelativePath = fs::relative(testObjectPath, compilationDirectory);
        testCompilationCommand.setOutput(testObjectPath);
        testCompilationCommand.setSourcePath(testSourcePath);

        declareTarget(testObjectPath, { testSourcePath.string() },
                      { testCompilationCommand.toStringWithChangingDirectory() });

        artifacts.push_back(testObjectPath);

        auto rootLinkUnitInfo = buildDatabase->getClientLinkUnitInfo(rootPath);
        fs::path testExecutablePath = getTestExecutablePath(sourcePath);

        std::vector<std::string> filesToLink{ "$(GTEST_MAIN)", "$(GTEST_ALL)", testObjectPath,
                                              sharedOutput.value() };
        if (rootLinkUnitInfo->commands[0].isArchiveCommand()) {
            std::vector<std::string> dynamicLinkCommandLine{ cxxLinker,          "$(LDFLAGS)",
                                                             pthreadFlag,        coverageLinkFlags,
                                                             sanitizerLinkFlags, "-o",
                                                             testExecutablePath };
            CollectionUtils::extend(dynamicLinkCommandLine, filesToLink);
            dynamicLinkCommandLine.push_back(
                getLibraryDirectoryFlag(sharedOutput.value().parent_path()));
            utbot::LinkCommand dynamicLinkCommand{ dynamicLinkCommandLine, buildDirectory };
            declareTarget(testExecutablePath, filesToLink,
                          { dynamicLinkCommand.toStringWithChangingDirectory() });
        } else {
            utbot::LinkCommand dynamicLinkCommand = rootLinkUnitInfo->commands[0];
            dynamicLinkCommand.setLinker(cxxLinker);
            dynamicLinkCommand.setOutput(testExecutablePath);
            dynamicLinkCommand.erase_if([&](std::string const &argument) {
                return CollectionUtils::contains(rootLinkUnitInfo->files, argument) ||
                       argument == SHARED_FLAG ||
                       StringUtils::startsWith(argument, libraryDirOption) ||
                       StringUtils::startsWith(argument, linkFlag);
            });
            for (std::string &argument : dynamicLinkCommand.getCommandLine()) {
                removeScriptFlag(argument);
                removeSonameFlag(argument);
            }
            dynamicLinkCommand.setOptimizationLevel(OPTIMIZATION_FLAG);
            dynamicLinkCommand.addFlagsToBegin(
                { pthreadFlag, coverageLinkFlags, sanitizerLinkFlags });
            dynamicLinkCommand.addFlagsToBegin(filesToLink);
            dynamicLinkCommand.addFlagToBegin(
                getLibraryDirectoryFlag(sharedOutput.value().parent_path()));
            dynamicLinkCommand.addFlagToBegin("$(LDFLAGS)");
            declareTarget(testExecutablePath, filesToLink,
                          { dynamicLinkCommand.toStringWithChangingDirectory() });
        }

        artifacts.push_back(testExecutablePath);
    }
    fs::path NativeMakefilePrinter::getTestExecutablePath(const fs::path &sourcePath) const {
        return Paths::removeExtension(
            Paths::removeExtension(Paths::getRecompiledFile(projectContext, sourcePath)));
    }

    static fs::path getAsanLibraryPath() {
        return Paths::getUTBotDebsInstallDir() / "usr/lib/gcc/x86_64-linux-gnu/9/libasan.so";
    }

    NativeMakefilePrinter::NativeMakefilePrinter(const NativeMakefilePrinter &baseMakefilePrinter,
                                                 const fs::path &sourcePath)
        : projectContext(baseMakefilePrinter.projectContext),
          buildDatabase(baseMakefilePrinter.buildDatabase),
          rootPath(baseMakefilePrinter.rootPath),
          primaryCompiler(baseMakefilePrinter.primaryCompiler),
          primaryCxxCompiler(baseMakefilePrinter.primaryCxxCompiler),
          primaryCompilerName(baseMakefilePrinter.primaryCompilerName),
          primaryCxxCompilerName(baseMakefilePrinter.primaryCxxCompilerName),
          cxxLinker(baseMakefilePrinter.cxxLinker), pthreadFlag(baseMakefilePrinter.pthreadFlag),
          coverageLinkFlags(baseMakefilePrinter.coverageLinkFlags),
          sanitizerLinkFlags(baseMakefilePrinter.sanitizerLinkFlags),
          buildDirectory(baseMakefilePrinter.buildDirectory),
          dependencyDirectory(baseMakefilePrinter.dependencyDirectory),
          artifacts(baseMakefilePrinter.artifacts), sharedOutput(baseMakefilePrinter.sharedOutput) {
        resetStream();

        ss << baseMakefilePrinter.ss.str();

        addTestTarget(sourcePath);

        fs::path testExecutablePath = getTestExecutablePath(sourcePath);

        auto rootLinkUnitInfo = buildDatabase->getClientLinkUnitInfo(rootPath);
        declareTarget("bin", { FORCE }, { stringFormat("echo %s", sharedOutput.value()) });

        utbot::RunCommand testRunCommand{ { testExecutablePath.string(), "$(GTEST_FLAGS)" },
                                          buildDirectory };
        testRunCommand.addEnvironmentVariable("PATH", "$$PATH:$(pwd)");
        if (primaryCompilerName == CompilationUtils::CompilerName::GCC) {
            testRunCommand.addEnvironmentVariable("LD_PRELOAD", getAsanLibraryPath().string() + ":${LD_PRELOAD}");
        }
        testRunCommand.addEnvironmentVariable(SanitizerUtils::UBSAN_OPTIONS_NAME,
                                              SanitizerUtils::UBSAN_OPTIONS_VALUE);
        testRunCommand.addEnvironmentVariable(SanitizerUtils::ASAN_OPTIONS_NAME,
                                              SanitizerUtils::ASAN_OPTIONS_VALUE);

        declareTarget("build", { testExecutablePath }, {});
        declareTarget("run", { FORCE, "build" },
                      { testRunCommand.toStringWithChangingDirectory() });

        close();
    }

    BuildResult
    NativeMakefilePrinter::addLinkTargetRecursively(const fs::path &unitFile,
                                                    const std::string &suffixForParentOfStubs,
                                                    bool hasParent) {
        if (CollectionUtils::contains(buildResults, unitFile)) {
            return buildResults.at(unitFile);
        }
        if (Paths::isObjectFile(unitFile)) {
            auto buildResult = addObjectFile(unitFile, suffixForParentOfStubs);
            return buildResults[unitFile] = buildResult;
        }

        auto linkUnitInfo = buildDatabase->getClientLinkUnitInfo(unitFile);
        BuildResult::Type unitType = BuildResult::Type::NONE;
        CollectionUtils::MapFileTo<fs::path> fileMapping;
        auto unitBuildResults = CollectionUtils::transformTo<std::vector<BuildResult>>(
            linkUnitInfo->files, [&](fs::path const &dependency) {
                BuildResult buildResult =
                    addLinkTargetRecursively(dependency, suffixForParentOfStubs, true);
                unitType |= buildResult.type;
                fileMapping[dependency] = buildResult.output;
                return buildResult;
            });

        auto dependencies = CollectionUtils::transformTo<CollectionUtils::FileSet>(
            unitBuildResults, [](BuildResult const &buildResult) { return buildResult.output; });

        bool isExecutable = !Paths::isLibraryFile(unitFile);
        fs::path recompiledFile =
            Paths::getRecompiledFile(projectContext, linkUnitInfo->getOutput());
        if (isExecutable || Paths::isSharedLibraryFile(unitFile)) {
            sharedOutput =
                getSharedLibrary(*linkUnitInfo, unitType, suffixForParentOfStubs);
            recompiledFile = sharedOutput.value();
        } else {
            recompiledFile =
                LinkerUtils::applySuffix(recompiledFile, unitType, suffixForParentOfStubs);
        }

        auto commandActions =
            CollectionUtils::transform(linkUnitInfo->commands, [&](utbot::LinkCommand linkCommand) {
                linkCommand.erase(STATIC_FLAG);
                linkCommand.setOutput(recompiledFile);
                for (std::string &argument : linkCommand.getCommandLine()) {
                    if (CollectionUtils::contains(linkUnitInfo->files, argument)) {
                        argument = fileMapping.at(argument);
                    }
                }
                if (!linkCommand.isArchiveCommand()) {
                    linkCommand.setLinker(CompilationUtils::getBundledCompilerPath(
                            CompilationUtils::getCompilerName(linkCommand.getLinker())));
                    std::vector<std::string> libraryDirectoriesFlags;
                    for (std::string &argument : linkCommand.getCommandLine()) {
                        removeScriptFlag(argument);
                        removeSonameFlag(argument);
                        auto optionalLibraryAbsolutePath =
                            getLibraryAbsolutePath(argument, linkCommand.getDirectory());
                        if (optionalLibraryAbsolutePath.has_value()) {
                            const fs::path &absolutePath = optionalLibraryAbsolutePath.value();
                            if (Paths::isSubPathOf(projectContext.buildDir, absolutePath)) {
                                fs::path recompiledDir =
                                    Paths::getRecompiledFile(projectContext, absolutePath);
                                string directoryFlag = getLibraryDirectoryFlag(recompiledDir);
                                libraryDirectoriesFlags.push_back(directoryFlag);
                            }
                        }
                    }
                    linkCommand.addFlagsToBegin(libraryDirectoriesFlags);
                    linkCommand.addFlagsToBegin({ "-Wl,--allow-multiple-definition",
                                                  coverageLinkFlags, sanitizerLinkFlags, "-Wl,--whole-archive" });
                    if (linkCommand.isSharedLibraryCommand()) {
                        linkCommand.addFlagToEnd(STUB_OBJECT_FILES);
                        dependencies.emplace(STUB_OBJECT_FILES);
                    }
                    linkCommand.addFlagToEnd("-Wl,--no-whole-archive");
                    linkCommand.setOptimizationLevel(OPTIMIZATION_FLAG);
                    linkCommand.addFlagToBegin("$(LDFLAGS)");
                }
                if (isExecutable) {
                    linkCommand.addFlagToBegin(SHARED_FLAG);
                }
                return linkCommand.toStringWithChangingDirectory();
            });
        std::string removeAction = stringFormat("rm -f %s", recompiledFile);
        std::vector<std::string> actions{ removeAction };
        CollectionUtils::extend(actions, std::move(commandActions));

        declareTarget(recompiledFile, dependencies, actions);

        artifacts.push_back(recompiledFile);

        if (!hasParent && Paths::isStaticLibraryFile(unitFile)) {
            sharedOutput =
                getSharedLibrary(*linkUnitInfo, unitType, suffixForParentOfStubs);
            std::vector<std::string> sharedLinkCommandLine{
                primaryCompiler,         "$(LDFLAGS)",   SHARED_FLAG,           coverageLinkFlags,
                sanitizerLinkFlags,      "-o",           sharedOutput.value(),
                "-Wl,--whole-archive",   recompiledFile, STUB_OBJECT_FILES,
                "-Wl,--no-whole-archive"
            };
            utbot::LinkCommand sharedLinkCommand{ sharedLinkCommandLine, buildDirectory };
            declareTarget(sharedOutput.value(), { recompiledFile, STUB_OBJECT_FILES },
                          { sharedLinkCommand.toStringWithChangingDirectory() });


            artifacts.push_back(sharedOutput.value());
        }
        return buildResults[unitFile] = { recompiledFile, unitType };
    }


    fs::path
    NativeMakefilePrinter::getSharedLibrary(const BuildDatabase::TargetInfo &linkUnitInfo,
                                            BuildResult::Type unitType,
                                            std::string const &suffixForParentOfStubs) {
        fs::path output = Paths::getRecompiledFile(projectContext, linkUnitInfo.getOutput());
        output = CompilationUtils::removeSharedLibraryVersion(output);
        fs::path sharedLibrary = Paths::isSharedLibraryFile(output)
                                     ? output
                                     : Paths::addPrefix(Paths::addExtension(output, ".so"), "lib");
        return LinkerUtils::applySuffix(sharedLibrary, unitType, suffixForParentOfStubs);
    }

    void NativeMakefilePrinter::addStubs(const CollectionUtils::FileSet &stubsSet) {
        auto stubObjectFiles = CollectionUtils::transformTo<CollectionUtils::FileSet>(
            Synchronizer::dropHeaders(stubsSet), [this](fs::path const &stub) {
                fs::path sourcePath = Paths::stubPathToSourcePath(projectContext, stub);
                fs::path stubBuildFilePath =
                    Paths::getStubBuildFilePath(projectContext, sourcePath);
                auto compilationUnitInfo = buildDatabase->getClientCompilationUnitInfo(sourcePath);
                fs::path output = Paths::getRecompiledFile(projectContext, stub);
                addCompileTarget(stub, output, *compilationUnitInfo);
                return output;
            });
        declareVariable(STUB_OBJECT_FILES_NAME, StringUtils::joinWith(stubObjectFiles, " "));
    }

    void NativeMakefilePrinter::close() {
        declareTarget("clean", {},
                      { stringFormat("rm -rf %s", StringUtils::joinWith(artifacts, " ")) });
        auto allDependencies = stringFormat("%s/%%.d", dependencyDirectory);
        auto includeDependencies = stringFormat("%s/*.d", dependencyDirectory);
        auto includeTemporaryDependencies = stringFormat("%s/*.Td", dependencyDirectory);
        ss << ".PRECIOUS: " << allDependencies << "\n";
        ss << allDependencies << ": ;\n";
        ss << "\n";
        ss << "-include " << includeTemporaryDependencies << " " << includeDependencies << "\n";

        ss << "\n";
    }

    BuildResult
    NativeMakefilePrinter::addLinkTargetRecursively(const fs::path &unitFile,
                                                    const string &suffixForParentOfStubs) {
        return addLinkTargetRecursively(unitFile, suffixForParentOfStubs, false);
    }
}
