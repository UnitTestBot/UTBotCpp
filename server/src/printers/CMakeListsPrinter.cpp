#include <utils/ArgumentsUtils.h>
#include "CMakeListsPrinter.h"
#include "utils/Copyright.h"
#include "utils/FileSystemUtils.h"
#include "Synchronizer.h"
#include "loguru.h"

namespace printer {
    void CMakeListsPrinter::addDiscoverTestDirective(const std::string &testTargetName) {
        ss << "gtest_discover_tests(" << testTargetName << ")" << NL;
    }

    void CMakeListsPrinter::addLibrary(const std::string &libraryName, bool isShared, const std::vector<fs::path> &sourceFiles) {
        if (sourceFiles.empty())
            return;
        ss << "add_library" << LB();
        ss << libraryName << NL;
        if (isShared)
            ss << LINE_INDENT() << "SHARED" << NL;
        for (auto &file : sourceFiles) {
            ss << LINE_INDENT() << file.string() << NL;
        }
        ss << RB() << NL;
    }

    void CMakeListsPrinter::write(const fs::path &path) {
        FileSystemUtils::writeToFile(path, ss.str());
    }

    void CMakeListsPrinter::addCopyrightHeader() {
        ss << Copyright::GENERATED_CMAKELISTS_FILE_HEADER << NL;
    }

    std::string CMakeListsPrinter::RB() {
        tabsDepth--;
        return LINE_INDENT() + ")\n";
    }

    std::string CMakeListsPrinter::LB(bool startsWithSpace) {
        tabsDepth++;
        return std::string(startsWithSpace ? " " : "") + "(\n" + LINE_INDENT();
    }

    void CMakeListsPrinter::addIncludeDirectoriesForTarget(const std::string &targetName,
                                                           const std::set<std::string> includePaths) {
        if (includePaths.empty())
            return;
        ss << "target_include_directories" << LB() << targetName << " PUBLIC" << NL;
        for (auto &includePath : includePaths) {
            ss << LINE_INDENT() << includePath << NL;
        }
        ss << RB() << NL;
    }

    void CMakeListsPrinter::addTargetLinkLibraries(const std::string &targetName,
                                                   const std::vector<std::string> &librariesNamesToLink) {
        if (librariesNamesToLink.empty())
            return;
        ss << "target_link_libraries" << LB() << targetName << NL;
        for (auto &lib : librariesNamesToLink) {
            ss << LINE_INDENT() << lib << NL;
        }
        ss << RB() << NL;
    }

    CMakeListsPrinter::CMakeListsPrinter() { addCopyrightHeader(); }

    void CMakeListsPrinter::addExecutable(const std::string &executableName, const std::vector<fs::path> &sourceFiles) {
        if (sourceFiles.empty())
            return;
        ss << "add_executable" << LB() << executableName << NL;
        for (auto &subFile : sourceFiles) {
            ss << LINE_INDENT() << subFile.string() << NL;
        }
        ss << RB() << NL;
    }
}
