/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "TestMakefilesPrinter.h"
#include "utils/FileSystemUtils.h"

#include <utility>
#include <utils/MakefileUtils.h>

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
            projectContext(testGen.projectContext),
            sharedMakefilePrinter(testGen, stubSources),
            objMakefilePrinter(testGen, stubSources) {
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

    void TestMakefilesPrinter::init() {
        sharedMakefilePrinter.init();
        objMakefilePrinter.init();
    }

    void TestMakefilesPrinter::addStubs(const CollectionUtils::FileSet &stubsSet) {
        sharedMakefilePrinter.addStubs(stubsSet);
        objMakefilePrinter.addStubs(stubsSet);
    }

    TestMakefilesContent TestMakefilesPrinter::GetMakefiles(const fs::path &path) const {
        printer::DefaultMakefilePrinter generalMakefilePrinter;
        fs::path generalMakefilePath = Paths::getMakefilePathFromSourceFilePath(projectContext, path);
        fs::path sharedMakefilePath = getMakefilePathForShared(generalMakefilePath);
        fs::path objMakefilePath = getMakefilePathForObject(generalMakefilePath);

        generalMakefilePrinter.declareTarget(FORCE, {}, {});

        generalMakefilePrinter.declareTarget("bin", {FORCE}, {
                StringUtils::joinWith(
                        MakefileUtils::getMakeCommand(sharedMakefilePath.string(), "bin", true),
                        " ")
        });
        generalMakefilePrinter.declareTarget("build", {FORCE}, {
                StringUtils::stringFormat("%s || %s",
                                          StringUtils::joinWith(MakefileUtils::getMakeCommand(
                                                  sharedMakefilePath.string(), "build", true), " "),
                                          StringUtils::joinWith(MakefileUtils::getMakeCommand(
                                                  objMakefilePath.string(), "build", true), " "))
        });
        generalMakefilePrinter.declareTarget("run", {FORCE}, {
                StringUtils::stringFormat("%s && { %s; exit $$?; } || { %s && { %s; exit $$?; } }",
                                          StringUtils::joinWith(MakefileUtils::getMakeCommand(
                                                  sharedMakefilePath.string(), "build", true), " "),
                                          StringUtils::joinWith(MakefileUtils::getMakeCommand(
                                                  sharedMakefilePath.string(), "run", true), " "),
                                          StringUtils::joinWith(MakefileUtils::getMakeCommand(
                                                  objMakefilePath.string(), "build", true), " "),
                                          StringUtils::joinWith(MakefileUtils::getMakeCommand(
                                                  objMakefilePath.string(), "run", true), " "))
        });
        generalMakefilePrinter.declareTarget("clean", {FORCE}, {
                StringUtils::joinWith(
                        MakefileUtils::getMakeCommand(sharedMakefilePath.string(), "clean", true), " "),
                StringUtils::joinWith(
                        MakefileUtils::getMakeCommand(objMakefilePath.string(), "clean", true), " ")
        });

        return {generalMakefilePath,
                generalMakefilePrinter.ss.str(),
                NativeMakefilePrinter(sharedMakefilePrinter, path).ss.str(),
                NativeMakefilePrinter(objMakefilePrinter, path).ss.str()};
    }
}