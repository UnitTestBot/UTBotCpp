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

    TestMakefilesPrinter::TestMakefilesPrinter(const BaseTestGen &testGen,
                                               CollectionUtils::FileSet const *stubSources) :
            TestMakefilesPrinter(
                    testGen,
                    testGen.buildDatabase->getTargetPath(),
                    CompilationUtils::getBundledCompilerPath(CompilationUtils::getCompilerName(
                            testGen.buildDatabase->compilationDatabase->getBuildCompilerPath())),
                    stubSources) {
    }

    TestMakefilesPrinter::TestMakefilesPrinter(
            const BaseTestGen &testGen,
//            utbot::ProjectContext projectContext,
//            std::shared_ptr<BuildDatabase> buildDatabase,
            fs::path const &rootPath,
            fs::path primaryCompiler,
            CollectionUtils::FileSet const *stubSources) :
            RelativeMakefilePrinter(Paths::getUtbotBuildDir(testGen.projectContext),
                                    Paths::getRelativeUtbotBuildDir(testGen.projectContext),
                                    testGen.projectContext.projectPath),
            projectContext(testGen.projectContext),
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

    TestMakefilesContent TestMakefilesPrinter::GetMakefiles(const fs::path &path) {
        printer::DefaultMakefilePrinter generalMakefilePrinter;
        fs::path generalMakefilePath = Paths::getMakefilePathFromSourceFilePath(projectContext, path);
        fs::path sharedMakefilePath = getMakefilePathForShared(generalMakefilePath);
        fs::path objMakefilePath = getMakefilePathForObject(generalMakefilePath);

        generalMakefilePrinter.ss << getProjectStructureRelativeTo(path);
        generalMakefilePrinter.ss << ss.str();

        generalMakefilePrinter.declareTarget(FORCE, {}, {});

        const std::string sharedMakefilePathRelative =
                sharedMakefilePrinter.getRelativePath(sharedMakefilePath);
        const std::string objMakefilePathRelative =
                objMakefilePrinter.getRelativePath(objMakefilePath);
        generalMakefilePrinter.declareTarget("bin", {FORCE}, {
                StringUtils::joinWith(
                        MakefileUtils::getMakeCommand(sharedMakefilePathRelative, "bin", true),
                        " ")
        });
        generalMakefilePrinter.declareTarget("build", {FORCE}, {
                StringUtils::stringFormat("%s || %s",
                                          StringUtils::joinWith(MakefileUtils::getMakeCommand(
                                                  sharedMakefilePathRelative, "build", true), " "),
                                          StringUtils::joinWith(MakefileUtils::getMakeCommand(
                                                  objMakefilePathRelative, "build", true), " "))
        });

        generalMakefilePrinter.declareTarget("run", {FORCE}, {
                StringUtils::stringFormat("%s && { %s; exit $$?; } || { %s && { %s; exit $$?; } }",
                                          StringUtils::joinWith(MakefileUtils::getMakeCommand(
                                                  sharedMakefilePathRelative, "build", true), " "),
                                          StringUtils::joinWith(MakefileUtils::getMakeCommand(
                                                  sharedMakefilePathRelative, "run", true), " "),
                                          StringUtils::joinWith(MakefileUtils::getMakeCommand(
                                                  objMakefilePathRelative, "build", true), " "),
                                          StringUtils::joinWith(MakefileUtils::getMakeCommand(
                                                  objMakefilePathRelative, "run", true), " "))
        });
        generalMakefilePrinter.declareTarget("clean", {FORCE}, {
                StringUtils::joinWith(
                        MakefileUtils::getMakeCommand(sharedMakefilePathRelative, "clean", true), " "),
                StringUtils::joinWith(
                        MakefileUtils::getMakeCommand(objMakefilePathRelative, "clean", true), " ")
        });

        return {generalMakefilePath,
                generalMakefilePrinter.ss.str(),
                NativeMakefilePrinter(sharedMakefilePrinter, path).ss.str(),
                NativeMakefilePrinter(objMakefilePrinter, path).ss.str()};
    }
}
