#include "ProjectContext.h"

#include <protobuf/testgen.grpc.pb.h>

#include <utility>

namespace utbot {
    ProjectContext::ProjectContext(std::string projectName,
                                   fs::path projectPath,
                                   fs::path testDirPath,
                                   fs::path buildDirRelativePath,
                                   fs::path clientProjectPath,
                                   fs::path itfPath)
            : projectName(std::move(projectName)), projectPath(std::move(projectPath)),
              testDirPath(std::move(testDirPath)),
              buildDirRelativePath(std::move(buildDirRelativePath)),
              clientProjectPath(std::move(clientProjectPath)),
              itfPath(std::move(itfPath)) {
    }

    ProjectContext::ProjectContext(const testsgen::ProjectContext &projectContext)
            : ProjectContext(projectContext.projectname(),
                             projectContext.projectpath(),
                             projectContext.testdirpath(),
                             projectContext.builddirrelativepath(),
                             projectContext.clientprojectpath(),
                             projectContext.itfpath()) {}

    ProjectContext::ProjectContext(const testsgen::SnippetRequest &request, fs::path serverBuildDir)
            : ProjectContext(request.projectcontext().projectname(), request.projectcontext().projectpath(),
                             request.projectcontext().testdirpath(), request.projectcontext().builddirrelativepath(),
                             request.projectcontext().clientprojectpath(),
                             request.projectcontext().itfpath()) {}

    fs::path ProjectContext::buildDir() const {
        return projectPath / buildDirRelativePath;
    }
}
