/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_LLVMCOVERAGETOOL_H
#define UNITTESTBOT_LLVMCOVERAGETOOL_H

#include "CoverageAndResultsGenerator.h"
#include "CoverageTool.h"

class LlvmCoverageTool : public CoverageTool {
public:
    LlvmCoverageTool(utbot::ProjectContext projectContext, ProgressWriter const *progressWriter);

    std::vector<BuildRunCommand> getBuildRunCommands(const std::vector<UnitTest> &testsToLaunch,
                                                     bool withCoverage) override;

    std::vector<ShellExecTask> getCoverageCommands(const std::vector<UnitTest> &testFilePath) override;

    [[nodiscard]] Coverage::CoverageMap getCoverageInfo() const override;
    [[nodiscard]] nlohmann::json getTotals() const override;
    void cleanCoverage() const override;
private:
    const utbot::ProjectContext projectContext;
    void countLineCoverage(Coverage::CoverageMap& coverageMap, const std::string& filename) const;
    void checkLineForPartial(Coverage::FileCoverage::SourceLine line, Coverage::FileCoverage& fileCoverage) const;
};


#endif // UNITTESTBOT_LLVMCOVERAGETOOL_H
