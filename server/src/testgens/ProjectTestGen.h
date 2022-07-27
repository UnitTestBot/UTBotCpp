#ifndef UNITTESTBOT_PROJECTTESTGEN_H
#define UNITTESTBOT_PROJECTTESTGEN_H

#include "BaseTestGen.h"

#include <optional>

class ProjectTestGen : public BaseTestGen {
public:
    ProjectTestGen(const testsgen::ProjectRequest &request,
                   ProgressWriter *progressWriter,
                   bool testMode,
                   bool autoSrcPaths = true);

    ~ProjectTestGen() override = default;

    std::string toString() override;

    const testsgen::ProjectRequest *getRequest() const;

//    void setTargetForSource(fs::path const &sourcePath) override;

private:
    testsgen::ProjectRequest const *const request;

    std::vector<fs::path> getRequestSourcePaths() const;

    void autoDetectSourcePathsIfNotEmpty();
};


#endif // UNITTESTBOT_PROJECTTESTGEN_H
