/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_GCOVCOVERAGETOOL_H
#define UNITTESTBOT_GCOVCOVERAGETOOL_H

#include "Coverage.h"
#include "CoverageAndResultsGenerator.h"
#include "CoverageTool.h"

class GcovCoverageTool : public CoverageTool {
public:
    GcovCoverageTool(utbot::ProjectContext projectContext, ProgressWriter const *progressWriter);

    std::vector<BuildRunCommand> getBuildRunCommands(const std::vector<UnitTest> &testsToLaunch,
                                                     bool withCoverage) override;

    std::vector <std::string> getGcovArguments(bool jsonFormat) const;

    std::vector<ShellExecTask> getCoverageCommands(const std::vector<UnitTest> &testsToLaunch) override;

    [[nodiscard]] Coverage::CoverageMap getCoverageInfo() const override;

    [[nodiscard]] nlohmann::json getTotals() const override;

    void cleanCoverage() const override;

private:
    const utbot::ProjectContext projectContext;

    std::vector<fs::path> getGcdaFiles() const;
};


#endif // UNITTESTBOT_GCOVCOVERAGETOOL_H
