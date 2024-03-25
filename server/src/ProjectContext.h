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
                   fs::path testDirPath,
                   fs::path buildDirRelativePath,
                   fs::path serverBuildDir,
                   fs::path itfPath);

    explicit ProjectContext(const testsgen::ProjectContext &projectContext);

    ProjectContext(const testsgen::SnippetRequest &request, fs::path serverBuildDir);

    [[nodiscard]] fs::path buildDir() const;

    const std::string projectName;
    const fs::path projectPath;
    const fs::path testDirPath;
    const fs::path buildDirRelativePath;
    const fs::path clientProjectPath;
    const fs::path itfPath;
};
}


#endif //UNITTESTBOT_PROJECTCONTEXT_H
