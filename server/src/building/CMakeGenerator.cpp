#include "Synchronizer.h"
#include "building/Linker.h"
#include "utils/FileSystemUtils.h"
#include "utils/Copyright.h"
#include <utils/ArgumentsUtils.h>
#include <utils/SanitizerUtils.h>
#include <filesystem>
#include "CMakeGenerator.h"

FileInfoForTransfer CMakeGenerator::getResult() {
    return {testGen->projectContext.testDirPath / "CMakeLists.txt", printer.ss.str()};
}

void CMakeGenerator::generate(const fs::path &target, const CollectionUtils::FileSet &stubsSet,
                              const CollectionUtils::FileSet &presentedFiles, const CollectionUtils::FileSet &stubSources) {
    generateCMakeForTargetRecursively(target, stubsSet, stubSources);
    addTests(presentedFiles, target, stubsSet);
    printer.write(testGen->projectContext.testDirPath / "CMakeLists.txt");
}

void CMakeGenerator::generateCMakeForTargetRecursively(const fs::path &target,
                                                       const CollectionUtils::FileSet &stubsSet,
                                                       const CollectionUtils::FileSet &stubSources) {
    addLinkTargetRecursively(target, true, stubsSet, stubSources);
}

std::shared_ptr<const BuildDatabase::TargetInfo> CMakeGenerator::getTargetUnitInfo(const fs::path &targetPath) {
    auto targetBuildDb = testGen->getTargetBuildDatabase();
    return targetBuildDb->getClientLinkUnitInfo(targetPath);
}

void CMakeGenerator::addLinkTargetRecursively(const fs::path &path, bool isRoot,
                                              const CollectionUtils::FileSet &stubsSet,
                                              const CollectionUtils::FileSet &stubSources) {
    if (CollectionUtils::contains(alreadyBuildFiles, path)) {
        return;
    }
    auto targetInfo = getTargetUnitInfo(path);
    std::vector<std::string> dependentLibs;
    std::vector<fs::path> dependentSourceFiles;
    for (auto &file: targetInfo->files) {
        if (Paths::isObjectFile(file)) {
            auto objectInfo = testGen->getClientCompilationUnitInfo(file);
            auto fileToAdd = objectInfo->getSourcePath();
            if (CollectionUtils::contains(stubSources, fileToAdd)) {
                fileToAdd = Paths::sourcePathToStubPath(testGen->projectContext, fileToAdd);
            } else if (Paths::isCFile(fileToAdd)) {
                // todo: wrappers help us to replace main to main_, what to do for c++ files?
                fileToAdd = Paths::getWrapperFilePath(testGen->projectContext, fileToAdd);
            }
            dependentSourceFiles.push_back(fileToAdd);
            alreadyBuildFiles.insert(file);
        } else {
            dependentLibs.push_back(file);
        }
    }
    auto isExecutable = !Paths::isLibraryFile(path);
    auto linkWithStubs = isRoot || isExecutable || Paths::isSharedLibraryFile(path);
    if (linkWithStubs) {
        for (auto &stubFile: stubsSet) {
            dependentSourceFiles.push_back(stubFile);
        }
    }
    auto libName = getLibraryName(path, isRoot);
    printer.addLibrary(libName, /*isShared=*/linkWithStubs,
                       CollectionUtils::transform(dependentSourceFiles, [&](const fs::path &file) {
                           return fs::relative(file, testGen->projectContext.testDirPath);
                       }));
    printer.addIncludeDirectoriesForTarget(libName, getIncludeDirectoriesFor(path));
    alreadyBuildFiles.insert(path);
    if (!dependentLibs.empty()) {
        for (auto &lib: dependentLibs) {
            addLinkTargetRecursively(lib, false, stubsSet, stubSources);
        }
        printer.addTargetLinkLibraries(libName, CollectionUtils::transformTo<std::vector<std::string>>(dependentLibs, [&](const fs::path &lib){
            return getLibraryName(lib, false);
        }));
    }
}

std::set<std::string> CMakeGenerator::getIncludeDirectoriesFor(const fs::path &target) {
    auto targetInfo = getTargetUnitInfo(target);
    std::set<std::string> res;
    for (auto &file: targetInfo->files) {
        if (!Paths::isObjectFile(file))
            continue;
        auto objectInfo = testGen->getClientCompilationUnitInfo(file);
        for (auto &arg: objectInfo->command.getCommandLine()) {
            if (StringUtils::startsWith(arg, "-I")) {
                res.insert(getAbsolutePath(arg.substr(2)).string());
            }
        }
    }
    return res;
}

std::filesystem::path CMakeGenerator::getAbsolutePath(const std::filesystem::path &path) {
    // std::path is used here because fs::path is normalized in constructor, and it leads to the following behaviour
    // ${cmake_path}/../a/b -> a/b, which is not what we want
    auto relativeFromProjectToPath = path.lexically_relative(testGen->projectContext.projectPath.string());
    auto relativeFromTestsToProject = std::filesystem::path(
            testGen->projectContext.projectPath.string()).lexically_relative(
            testGen->projectContext.testDirPath.string());
    auto res = std::filesystem::path("${CMAKE_CURRENT_SOURCE_DIR}") / relativeFromTestsToProject /
               relativeFromProjectToPath;
    return res;
}

std::string CMakeGenerator::getLibraryName(const fs::path &lib, bool isRoot) {
    return lib.stem().string() +
           (isRoot && Paths::isStaticLibraryFile(lib) ? "_shared" : "") + "_utbot";
}

void CMakeGenerator::addTests(const CollectionUtils::FileSet &filesUnderTest, const fs::path &target,
                              const CollectionUtils::FileSet &stubSet) {
    std::vector<fs::path> testFiles;
    for (auto &file: filesUnderTest) {
        testFiles.push_back(Paths::sourcePathToTestPath(testGen->projectContext, file));
    }
    auto testsTargetName = "utbot_tests";
    printer.addExecutable(testsTargetName,
                          CollectionUtils::transformTo<std::vector<fs::path>>(filesUnderTest,
            [&](const fs::path &elem) {
        return fs::relative(Paths::sourcePathToTestPath(testGen->projectContext, elem), testGen->projectContext.testDirPath); }));
    printer.addTargetLinkLibraries(testsTargetName, {GTEST_TARGET_NAME, getRootLibraryName(target)});
    printer.addDiscoverTestDirective(testsTargetName);
}

std::string CMakeGenerator::getRootLibraryName(const fs::path &path) {
    return getLibraryName(path, true);
}

CMakeGenerator::CMakeGenerator(const BaseTestGen *testGen) : testGen(testGen), printer(printer::CMakeListsPrinter()) {}
