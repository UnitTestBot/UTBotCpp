#ifndef UTBOTCPP_PROJECTBUILDDATABASE_H
#define UTBOTCPP_PROJECTBUILDDATABASE_H

#include "BuildDatabase.h"

class ProjectBuildDatabase : public BuildDatabase {
private:
    void initObjects(const nlohmann::json &compileCommandsJson);

    void initInfo(const nlohmann::json &linkCommandsJson, bool skipObjectWithoutSource);

    void filterInstalledFiles();

    void addLocalSharedLibraries();

    void fillTargetInfoParents();

public:
    ProjectBuildDatabase(fs::path buildCommandsJsonPath, fs::path serverBuildDir,
                         utbot::ProjectContext projectContext, bool skipObjectWithoutSource);

    explicit ProjectBuildDatabase(utbot::ProjectContext projectContext, bool skipObjectWithoutSource);
};


#endif //UTBOTCPP_PROJECTBUILDDATABASE_H
