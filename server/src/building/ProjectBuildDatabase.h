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
    ProjectBuildDatabase(fs::path buildCommandsJsonPath, fs::path serverBuildDir,
                         utbot::ProjectContext projectContext);

    ProjectBuildDatabase(utbot::ProjectContext projectContext);
};


#endif //UTBOTCPP_PROJECTBUILDDATABASE_H
