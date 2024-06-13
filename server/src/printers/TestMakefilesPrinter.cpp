#include "TestMakefilesPrinter.h"
#include "utils/FileSystemUtils.h"

#include <utility>
#include <utils/MakefileUtils.h>
#include "utils/StringUtils.h"
#include "RelativeMakefilePrinter.h"

namespace printer {
    fs::path getMakefilePathForShared(fs::path path) {
        return Paths::addSuffix(Paths::addPrefix(path, "__"), "_shared");
    }

    fs::path getMakefilePathForObject(fs::path path) {
        return Paths::addSuffix(Paths::addPrefix(path, "__"), "_obj");
    }

    void TestMakefilesContent::write() const {
        FileSystemUtils::writeToFile(path, generalMakefileContent);
        fs::path sharedMakefilePath = getMakefilePathForShared(path);
        FileSystemUtils::writeToFile(sharedMakefilePath, sharedMakefileContent);
        fs::path objMakefilePath = getMakefilePathForObject(path);
        FileSystemUtils::writeToFile(objMakefilePath, objMakefileContent);
    }

    TestMakefilesContent::TestMakefilesContent(fs::path path, std::string generalMakefileStr,
                                               std::string sharedMakefileStr,
                                               std::string objMakefileStr) :
            path(std::move(path)), generalMakefileContent(std::move(generalMakefileStr)),
            sharedMakefileContent(std::move(sharedMakefileStr)), objMakefileContent(std::move(objMakefileStr)) {
    }

    TestMakefilesPrinter::TestMakefilesPrinter(const BaseTestGen *testGen,
                                               CollectionUtils::FileSet const *stubSources) :
            TestMakefilesPrinter(
                    testGen,
                    testGen->getTargetBuildDatabase()->getTargetPath(),
                    CompilationUtils::getBundledCompilerPath(CompilationUtils::getCompilerName(
                            testGen->getTargetBuildDatabase()->compilationDatabase->getBuildCompilerPath())),
                    stubSources) {
    }

    TestMakefilesPrinter::TestMakefilesPrinter(
            const BaseTestGen *testGen,
            fs::path const &rootPath,
            fs::path primaryCompiler,
            CollectionUtils::FileSet const *stubSources) :
            RelativeMakefilePrinter(Paths::getUTBotBuildDir(testGen->projectContext),
                                    Paths::getRelativeUtbotBuildDir(testGen->projectContext),
                                    testGen->projectContext.projectPath,
                                    testGen->projectContext.getTestDirAbsPath()),
            projectContext(testGen->projectContext),
            sharedMakefilePrinter(testGen, rootPath, primaryCompiler, stubSources, pathToShellVariable),
            objMakefilePrinter(testGen, rootPath, primaryCompiler, stubSources, pathToShellVariable) {
    }

    void
    TestMakefilesPrinter::addLinkTargetRecursively(const fs::path &unitFile, const std::string &suffixForParentOfStubs) {
        sharedMakefilePrinter.addLinkTargetRecursively(unitFile, suffixForParentOfStubs, true);
        objMakefilePrinter.addLinkTargetRecursively(unitFile, suffixForParentOfStubs, false);
    }

    void TestMakefilesPrinter::close() {
        sharedMakefilePrinter.close();
        objMakefilePrinter.close();
    }

    void TestMakefilesPrinter::addStubs(const CollectionUtils::FileSet &stubsSet) {
        sharedMakefilePrinter.addStubs(stubsSet);
        objMakefilePrinter.addStubs(stubsSet);
    }

    TestMakefilesContent TestMakefilesPrinter::GetMakefiles(const fs::path &sourcePath) {
        printer::DefaultMakefilePrinter generalMakefilePrinter;
        fs::path generalMakefilePath = Paths::getMakefilePathFromSourceFilePath(projectContext, sourcePath);
        fs::path sharedMakefilePath = getMakefilePathForShared(generalMakefilePath);
        fs::path objMakefilePath = getMakefilePathForObject(generalMakefilePath);

        generalMakefilePrinter.ss << getProjectStructureRelativeTo(generalMakefilePath);
        generalMakefilePrinter.ss << ss.str();

        const std::string sharedMakefilePathRelative =
                sharedMakefilePrinter.getRelativePath(sharedMakefilePath);
        const std::string objMakefilePathRelative =
                objMakefilePrinter.getRelativePath(objMakefilePath);
        generalMakefilePrinter.declareTarget("bin", {TARGET_FORCE}, {
                StringUtils::joinWith(
                        MakefileUtils::getMakeCommand(sharedMakefilePathRelative, "bin", true),
                        " ")
        });

        generalMakefilePrinter.declareTarget("compile_test", {TARGET_FORCE}, {
                StringUtils::joinWith(
                        MakefileUtils::getMakeCommand(sharedMakefilePathRelative, "compile_test", true),
                        " ")
        });

        generalMakefilePrinter.declareTarget(TARGET_BUILD, {TARGET_FORCE}, {
                StringUtils::stringFormat("%s || %s",
                                          StringUtils::joinWith(MakefileUtils::getMakeCommand(
                                                  sharedMakefilePathRelative, TARGET_BUILD, true), " "),
                                          StringUtils::joinWith(MakefileUtils::getMakeCommand(
                                                  objMakefilePathRelative, TARGET_BUILD, true), " "))
        });

        generalMakefilePrinter.declareTarget(TARGET_RUN, {TARGET_FORCE}, {
                StringUtils::stringFormat("%s && { %s; exit $$?; } || { %s && { %s; exit $$?; } }",
                                          StringUtils::joinWith(MakefileUtils::getMakeCommand(
                                                  sharedMakefilePathRelative, TARGET_BUILD, true), " "),
                                          StringUtils::joinWith(MakefileUtils::getMakeCommand(
                                                  sharedMakefilePathRelative, TARGET_RUN, true), " "),
                                          StringUtils::joinWith(MakefileUtils::getMakeCommand(
                                                  objMakefilePathRelative, TARGET_BUILD, true), " "),
                                          StringUtils::joinWith(MakefileUtils::getMakeCommand(
                                                  objMakefilePathRelative, TARGET_RUN, true), " "))
        });
        generalMakefilePrinter.declareTarget("clean", {TARGET_FORCE}, {
                StringUtils::joinWith(
                        MakefileUtils::getMakeCommand(sharedMakefilePathRelative, "clean", true), " "),
                StringUtils::joinWith(
                        MakefileUtils::getMakeCommand(objMakefilePathRelative, "clean", true), " ")
        });

        return {generalMakefilePath,
                generalMakefilePrinter.ss.str(),
                NativeMakefilePrinter(sharedMakefilePrinter, sourcePath).ss.str(),
                NativeMakefilePrinter(objMakefilePrinter, sourcePath).ss.str()};
    }
}
