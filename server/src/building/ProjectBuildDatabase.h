#ifndef UTBOTCPP_PROJECTBUILDDATABASE_H
#define UTBOTCPP_PROJECTBUILDDATABASE_H

#include "BuildDatabase.h"

class ProjectBuildDatabase : BuildDatabase {
private:
    ProjectBuildDatabase(fs::path _buildCommandsJsonPath, fs::path _serverBuildDir,
                         utbot::ProjectContext _projectContext);

public:
    static std::shared_ptr<ProjectBuildDatabase> create(const utbot::ProjectContext &projectContext);
    bool hasAutoTarget() const override;
};


#endif //UTBOTCPP_PROJECTBUILDDATABASE_H
