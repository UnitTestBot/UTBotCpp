#ifndef UNITTESTBOT_PROJECTCONTEXT_H
#define UNITTESTBOT_PROJECTCONTEXT_H


#include "utils/path/FileSystemPath.h"
#include <string>

namespace testsgen {
    class ProjectContext;

    class SnippetRequest;
}

namespace utbot {
    class ProjectContext {
    public:
        ProjectContext(std::string projectName,
                       fs::path projectPath,
                       fs::path clientProjectPath,
                       fs::path testDirRelPath,
                       fs::path reportDirRelPath,
                       fs::path buildDirRelPath,
                       fs::path itfRelPath);

        explicit ProjectContext(const testsgen::ProjectContext &projectContext);

        ProjectContext(const testsgen::SnippetRequest &request, fs::path serverBuildDir);

        [[nodiscard]] fs::path getTestDirAbsPath() const;

        [[nodiscard]] fs::path getReportDirAbsPath() const;

        [[nodiscard]] fs::path getBuildDirAbsPath() const;

        [[nodiscard]] bool hasItfPath() const;

        [[nodiscard]] fs::path getItfAbsPath() const;

        const std::string projectName;
        const fs::path projectPath;
        const fs::path clientProjectPath;

    private:
        const fs::path testDirRelPath;
        const fs::path reportDirRelPath;
        const fs::path buildDirRelPath;
        const fs::path itfRelPath;
    };
}


#endif //UNITTESTBOT_PROJECTCONTEXT_H
