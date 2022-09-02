#ifndef UTBOTCPP_TARGETBUILDDATABASE_H
#define UTBOTCPP_TARGETBUILDDATABASE_H

#include "BuildDatabase.h"


class TargetBuildDatabase : public BuildDatabase {
private:
    fs::path target;
    bool isAutoTarget;

public:
    TargetBuildDatabase(BuildDatabase *baseBuildDatabase, const std::string &targetOrSourcePath);

    bool hasAutoTarget() const;

    fs::path getTargetPath() const;

    std::vector<std::shared_ptr<TargetInfo>> getRootTargets() const override;

    std::vector<fs::path> getTargetPathsForSourceFile(const fs::path &sourceFilePath) const override;

    std::vector<fs::path> getTargetPathsForObjectFile(const fs::path &objectFile) const override;
};

#endif //UTBOTCPP_TARGETBUILDDATABASE_H
