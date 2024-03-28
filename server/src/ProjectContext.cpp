#include "ProjectContext.h"

#include <protobuf/testgen.grpc.pb.h>

#include <utility>

namespace utbot {
    ProjectContext::ProjectContext(std::string projectName,
                                   fs::path projectPath,
                                   fs::path clientProjectPath,
                                   fs::path testDirRelPath,
                                   fs::path reportDirRelPath,
                                   fs::path buildDirRelPath,
                                   fs::path itfRelPath)
            : projectName(std::move(projectName)),
              clientProjectPath(std::move(clientProjectPath)),
              projectPath(std::move(projectPath)),
              testDirRelPath(std::move(testDirRelPath)),
              reportDirRelPath(std::move(reportDirRelPath)),
              buildDirRelPath(std::move(buildDirRelPath)),
              itfRelPath(std::move(itfRelPath)) {
    }

    ProjectContext::ProjectContext(const testsgen::ProjectContext &projectContext)
            : ProjectContext(projectContext.projectname(),
                             projectContext.projectpath(),
                             projectContext.clientprojectpath(),
                             projectContext.testdirrelpath(),
                             projectContext.reportdirrelpath(),
                             projectContext.builddirrelpath(),
                             projectContext.itfrelpath()) {}

    ProjectContext::ProjectContext(const testsgen::SnippetRequest &request, fs::path serverBuildDir)
            : ProjectContext(request.projectcontext().projectname(),
                             request.projectcontext().projectpath(),
                             request.projectcontext().clientprojectpath(),
                             request.projectcontext().testdirrelpath(),
                             request.projectcontext().reportdirrelpath(),
                             request.projectcontext().builddirrelpath(),
                             request.projectcontext().itfrelpath()) {}

    fs::path ProjectContext::getTestDirAbsPath() const {
        return projectPath / testDirRelPath;
    }

    fs::path ProjectContext::getReportDirAbsPath() const {
        return projectPath / reportDirRelPath;
    }

    fs::path ProjectContext::getBuildDirAbsPath() const {
        return projectPath / buildDirRelPath;
    }

    bool ProjectContext::hasItfPath() const {
        return !itfRelPath.string().empty();
    }

    fs::path ProjectContext::getItfAbsPath() const {
        return projectPath / itfRelPath;
    }
}
