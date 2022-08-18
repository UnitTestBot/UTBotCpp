#ifndef UTBOTCPP_PROJECTBUILDDATABASE_H
#define UTBOTCPP_PROJECTBUILDDATABASE_H

#include "BuildDatabase.h"

class ProjectBuildDatabase : public BuildDatabase {
private:
    void initObjects(const nlohmann::json &compileCommandsJson);

    void initInfo(const nlohmann::json &linkCommandsJson);

    void filterInstalledFiles();

    void addLocalSharedLibraries();

    void fillTargetInfoParents();

public:
    ProjectBuildDatabase(fs::path _buildCommandsJsonPath, fs::path _serverBuildDir,
                         utbot::ProjectContext _projectContext);

    static std::shared_ptr<ProjectBuildDatabase> create(const utbot::ProjectContext &projectContext);
};


#endif //UTBOTCPP_PROJECTBUILDDATABASE_H
