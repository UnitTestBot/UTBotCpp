#include "ProjectContext.h"

#include <protobuf/testgen.grpc.pb.h>

#include <utility>

namespace utbot {
    ProjectContext::ProjectContext(std::string projectName,
                                   fs::path projectPath,
                                   fs::path testDirPath,
                                   fs::path buildDirRelativePath)
            : projectName(std::move(projectName)), projectPath(std::move(projectPath)),
              testDirPath(std::move(testDirPath)),
              buildDirRelativePath(std::move(buildDirRelativePath)) {}

    ProjectContext::ProjectContext(const testsgen::ProjectContext &projectContext)
        : ProjectContext(projectContext.projectname(),
                         projectContext.projectpath(),
                         projectContext.testdirpath(),
                         projectContext.builddirrelativepath()) {}

    ProjectContext::ProjectContext(const testsgen::SnippetRequest &request, fs::path serverBuildDir)
        : projectName(request.projectcontext().projectname()),
          projectPath(request.projectcontext().projectpath()),
          testDirPath(request.projectcontext().testdirpath()),
          buildDirRelativePath(request.projectcontext().builddirrelativepath()) { }

    fs::path ProjectContext::buildDir() const {
        return projectPath / buildDirRelativePath;
    }
}
