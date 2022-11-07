//
// Created by Арсений Волынец on 13.09.2022.
//

#ifndef UTBOTCPP_CMAKEGENERATOR_H
#define UTBOTCPP_CMAKEGENERATOR_H

#include "testgens/BaseTestGen.h"
#include "BuildResult.h"
#include "printers/CMakeListsPrinter.h"

#include <string>
#include <vector>
#include <building/Linker.h>

class CMakeGenerator {
public:
    CMakeGenerator() = delete;
    CMakeGenerator(const CMakeGenerator &other) = delete;
    explicit CMakeGenerator(const BaseTestGen* testGen);

    static inline std::string GTEST_TARGET_NAME = "GTest::gtest_main";

    CollectionUtils::FileSet alreadyBuildFiles;
    printer::CMakeListsPrinter printer;

    void generate(const fs::path &target, const CollectionUtils::FileSet &stubsSet,
                  const CollectionUtils::FileSet &presentedFiles,
                  const CollectionUtils::FileSet &stubSources);

    FileInfoForTransfer getResult();

    void addLinkTargetRecursively(const fs::path &path, bool isRoot, const CollectionUtils::FileSet &stubsSet, const CollectionUtils::FileSet &stubSourcesFromProject);

    void addTests(const CollectionUtils::FileSet &filesUnderTest, const fs::path &target,
             const CollectionUtils::FileSet &stubSet);

    void generateCMakeForTargetRecursively(const fs::path &target, const CollectionUtils::FileSet& stubsSet, const CollectionUtils::FileSet &stubSources);

    std::set<std::string> getIncludeDirectoriesFor(const fs::path &target);

    std::string getLibraryName(const fs::path &lib, bool isRoot);

    std::string getRootLibraryName(const fs::path &path);

    std::shared_ptr<const BuildDatabase::TargetInfo> getTargetUnitInfo(const fs::path &targetPath);

    const BaseTestGen *testGen;

private:
    std::filesystem::path getAbsolutePath(const std::filesystem::path &path);
};

#endif //UTBOTCPP_CMAKEGENERATOR_H
