#include "ProjectContext.h"

#include <protobuf/testgen.grpc.pb.h>

#include <utility>

namespace utbot {
    ProjectContext::ProjectContext(std::string projectName,
                                   fs::path projectPath,
                                   fs::path testDirPath,
                                   fs::path buildDirRelativePath,
                                   fs::path clientProjectPath)
            : projectName(std::move(projectName)), projectPath(std::move(projectPath)),
              testDirPath(std::move(testDirPath)),
              buildDirRelativePath(std::move(buildDirRelativePath)),
              clientProjectPath(clientProjectPath) {}

    ProjectContext::ProjectContext(const testsgen::ProjectContext &projectContext)
        : ProjectContext(projectContext.projectname(),
                         projectContext.projectpath(),
                         projectContext.testdirpath(),
                         projectContext.builddirrelativepath(),
                         projectContext.clientprojectpath()) {}

    ProjectContext::ProjectContext(const testsgen::SnippetRequest &request, fs::path serverBuildDir)
        : projectName(request.projectcontext().projectname()),
          projectPath(request.projectcontext().projectpath()),
          testDirPath(request.projectcontext().testdirpath()),
          buildDirRelativePath(request.projectcontext().builddirrelativepath()),
          clientProjectPath(request.projectcontext().clientprojectpath()) {}

    fs::path ProjectContext::buildDir() const {
        return projectPath / buildDirRelativePath;
    }
}
