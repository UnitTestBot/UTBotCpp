#ifndef UTBOTCPP_TARGETBUILDDATABASE_H
#define UTBOTCPP_TARGETBUILDDATABASE_H

#include "BuildDatabase.h"


class TargetBuildDatabase : BuildDatabase {
private:
    TargetBuildDatabase(BuildDatabase &baseBuildDatabase, const std::string &_target);

    void filterInstalledFiles();

    void addLocalSharedLibraries();

    void fillTargetInfoParents();

    void initObjects(const nlohmann::json &compileCommandsJson);

    void initInfo(const nlohmann::json &linkCommandsJson);

    fs::path target;
    bool isAutoTarget;
public:
    std::shared_ptr<TargetBuildDatabase> createForSourceOrTarget(std::shared_ptr<BuildDatabase> baseBuildDatabase,
                                                                 std::string &_targetOrSourcePath);

    bool hasAutoTarget() const override;

    std::vector<fs::path> getTargetPathsForSourceFile(const fs::path &sourceFilePath) const override;

    std::vector<fs::path> getTargetPathsForObjectFile(const fs::path &objectFile) const override;
};


#endif //UTBOTCPP_TARGETBUILDDATABASE_H
