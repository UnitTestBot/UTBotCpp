#ifndef UNITTESTBOT_TESTMAKEFILESPRINTER_H
#define UNITTESTBOT_TESTMAKEFILESPRINTER_H

#include "NativeMakefilePrinter.h"
#include "RelativeMakefilePrinter.h"

namespace printer {
    class TestMakefilesContent {
    private:
        fs::path path;
        std::string generalMakefileContent;
        std::string sharedMakefileContent;
        std::string objMakefileContent;

    public:
        TestMakefilesContent(fs::path path, std::string generalMakefileStr,
                             std::string sharedMakefileStr, std::string objMakefileStr);

        void write() const;
    };

    class TestMakefilesPrinter: public RelativeMakefilePrinter {
    private:
        utbot::ProjectContext projectContext;
        printer::NativeMakefilePrinter sharedMakefilePrinter;
        printer::NativeMakefilePrinter objMakefilePrinter;

    public:
        TestMakefilesPrinter(const BaseTestGen &testGen,
                             CollectionUtils::FileSet const *stubSources);

        TestMakefilesPrinter(
                utbot::ProjectContext projectContext,
                std::shared_ptr<BuildDatabase> buildDatabase,
                fs::path const &rootPath,
                fs::path primaryCompiler,
                CollectionUtils::FileSet const *stubSources);

        void close();

        void addLinkTargetRecursively(const fs::path &unitFile, const std::string &suffixForParentOfStubs);

        void addStubs(const CollectionUtils::FileSet &stubsSet);

        [[nodiscard]] TestMakefilesContent GetMakefiles(const fs::path &path);
    };
}

#endif //UNITTESTBOT_TESTMAKEFILESPRINTER_H
