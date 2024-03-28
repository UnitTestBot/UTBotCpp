#include "ProjectContext.h"

#include <protobuf/testgen.grpc.pb.h>

#include <utility>

namespace utbot {
    ProjectContext::ProjectContext(std::string projectName,
                                   fs::path projectPath,
                                   fs::path testDirPath,
                                   fs::path reportDirPath,
                                   fs::path buildDirRelPath,
                                   fs::path clientProjectPath,
                                   fs::path itfRelPath)
            : projectName(std::move(projectName)), projectPath(std::move(projectPath)),
              testDirPath(std::move(testDirPath)),
              reportDirPath(std::move(reportDirPath)),
              buildDirRelPath(std::move(buildDirRelPath)),
              clientProjectPath(std::move(clientProjectPath)),
              itfRelPath(std::move(itfRelPath)) {
    }

    ProjectContext::ProjectContext(const testsgen::ProjectContext &projectContext)
            : ProjectContext(projectContext.projectname(),
                             projectContext.projectpath(),
                             projectContext.testdirpath(),
                             projectContext.reportdirpath(),
                             projectContext.builddirrelpath(),
                             projectContext.clientprojectpath(),
                             projectContext.itfrelpath()) {}

    ProjectContext::ProjectContext(const testsgen::SnippetRequest &request, fs::path serverBuildDir)
            : ProjectContext(request.projectcontext().projectname(), request.projectcontext().projectpath(),
                             request.projectcontext().testdirpath(), request.projectcontext().reportdirpath(),
                             request.projectcontext().builddirrelpath(),
                             request.projectcontext().clientprojectpath(),
                             request.projectcontext().itfrelpath()) {}

    fs::path ProjectContext::buildDir() const {
        return projectPath / buildDirRelPath;
    }
}
