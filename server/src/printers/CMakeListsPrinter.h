#ifndef UTBOTCPP_CMAKELISTSPRINTER_H
#define UTBOTCPP_CMAKELISTSPRINTER_H

#include "Printer.h"
#include "testgens/BaseTestGen.h"
#include "BuildResult.h"

#include <string>
#include <vector>
#include <building/Linker.h>


namespace printer {
    class CMakeListsPrinter {
    public:
        CMakeListsPrinter();

        std::string SANITIZER_FLAGS_VAR_NAME="SANITIZER_FLAGS";
        std::string SANITIZER_LINK_FLAGS_VAR_NAME="SANITIZER_LINK_FLAGS";
        std::string COVERAGE_LINK_FLAGS_VAR_NAME="COVERAGE_LINK_FLAGS";
        using std_path = std::filesystem::path;


        std::string LB(bool startsWithSpace = false);

        std::string RB();

        static inline std::string GTEST_TARGET_NAME = "GTest::gtest_main";

        std::stringstream ss;
        int tabsDepth = 0;

        using FileToObjectInfo = CollectionUtils::MapFileTo<std::shared_ptr<const BuildDatabase::ObjectFileInfo>>;

        inline std::string LINE_INDENT() const {
            return StringUtils::repeat(TAB, tabsDepth);
        }

        void addTargetLinkLibraries(const std::string &targetName, const std::vector<std::string>& librariesNamesToLink);

        void addDiscoverTestDirective(const std::string &testTargetName);

        void addExecutable(const std::string &executableName, const std::vector<fs::path> &sourceFiles);

        void addIncludeDirectoriesForTarget(const std::string &targetName, const std::set<std::string> includePaths);

        void addLibrary(const std::string &libraryName, bool isShared, const std::vector<fs::path> &sourceFiles);

        void addLinkTargetRecursively(const fs::path &path, bool isRoot, const CollectionUtils::FileSet &stubsSet);

        void
        addTests(const CollectionUtils::FileSet &filesUnderTest, const fs::path &target,
                 const CollectionUtils::FileSet &stubSet);

        void generateCMakeForTargetRecursively(const fs::path &target, const CollectionUtils::FileSet& stubsSet);

        void generateCMakeLists(const CollectionUtils::FileSet &testsSourcePaths);

        std_path getAbsolutePath(const fs::path &path);

        std::set<std::string> getIncludeDirectoriesFor(const fs::path &target);

        std::string getLibraryName(const fs::path &lib, bool isRoot);

        fs::path getRelativePath(const fs::path &path);

        std::string getRootLibraryName(const fs::path &path);

        std::shared_ptr<const BuildDatabase::TargetInfo> getTargetUnitInfo(const fs::path &targetPath);

        void setLinkOptionsForTarget(const std::string &targetName, const std::string &options);

        const BaseTestGen *testGen;

        std::string wrapCMakeVariable(const std::string &variableName);

        void write(const fs::path &path);

    protected:
        void addCopyrightHeader();

    private:
        fs::path getTargetCmakePath(const fs::path &lib);
        void addInclude(const fs::path &cmakeFileToInclude);

        void addOptionsForSourceFiles(const FileToObjectInfo &sourceFiles);

        void addLinkFlagsForLibrary(const std::string &targetName, const fs::path &targetPath, bool transformExeToLib = false);
        fs::path getCMakeFileForTestFile(const fs::path &testFile);

        std::string getTestName(const fs::path &test);

        void addTestExecutable(const fs::path &path, const CollectionUtils::FileSet &stubs, const fs::path &target);

        void addLinkOptionsForTestTarget(const std::string &testName, const fs::path &target);

        void addVariable(const std::string &varName, const std::string &value);

        void tryChangeToAbsolute(std::string &argument);


        void setCompileOptionsForSource(const fs::path &sourceFile, const std::string &options);

        std::list<std::string> prepareCompileFlagsForTestFile(const fs::path &sourcePath);

    };
}

#endif //UTBOTCPP_CMAKELISTSPRINTER_H
