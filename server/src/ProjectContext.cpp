#include "ProjectContext.h"

#include <protobuf/testgen.grpc.pb.h>

#include <utility>

namespace utbot {
    ProjectContext::ProjectContext(std::string projectName,
                                   fs::path projectPath,
                                   fs::path testDirPath,
                                   fs::path buildDirRelativePath,
                                   fs::path resultsDirRelativePath)
        : projectName(std::move(projectName)),
          projectPath(std::move(projectPath)),
          testDirPath(std::move(testDirPath)),
          buildDirRelativePath(std::move(buildDirRelativePath)),
          buildDir(this->projectPath / this->buildDirRelativePath),
          resultsDirRelativePath(std::move(resultsDirRelativePath)),
          resultsDirPath(this->projectPath / this->resultsDirRelativePath) { }

    ProjectContext::ProjectContext(std::string projectName,
                                   fs::path projectPath,
                                   fs::path testDirPath,
                                   fs::path buildDirRelativePath)
            : ProjectContext(projectName, projectPath, testDirPath, buildDirRelativePath, "utbot-results") { }

    ProjectContext::ProjectContext(const testsgen::ProjectContext &projectContext)
        : ProjectContext(projectContext.projectname(),
                         projectContext.projectpath(),
                         projectContext.testdirpath(),
                         projectContext.builddirrelativepath(),
                         !projectContext.resultsdirrelativepath().empty() ?
                            projectContext.resultsdirrelativepath() :
                            "utbot-results") {}

    ProjectContext::ProjectContext(const testsgen::SnippetRequest &request, fs::path serverBuildDir)
        : projectName(request.projectcontext().projectname()),
          projectPath(request.projectcontext().projectpath()),
          testDirPath(request.projectcontext().testdirpath()),
          buildDirRelativePath(request.projectcontext().builddirrelativepath()),
          buildDir(std::move(serverBuildDir)) { }
}
