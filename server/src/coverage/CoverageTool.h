#ifndef UNITTESTBOT_COVERAGETOOL_H
#define UNITTESTBOT_COVERAGETOOL_H

#include "Coverage.h"
#include "ProjectContext.h"
#include "UnitTest.h"
#include "streams/IStreamWriter.h"
#include "streams/WriterUtils.h"
#include "utils/MakefileUtils.h"

#include <string>
#include <vector>

struct BuildRunCommand {
    UnitTest unitTest;
    MakefileUtils::MakefileCommand buildCommand;
    MakefileUtils::MakefileCommand runCommand;
};

class CoverageTool {
protected:
    ProgressWriter const *progressWriter;
    const utbot::ProjectContext projectContext;

    [[nodiscard]] std::string getGTestFlags(const UnitTest &unitTest) const;

public:
    CoverageTool(utbot::ProjectContext projectContext, ProgressWriter const *progressWriter);

    [[nodiscard]] virtual std::vector<BuildRunCommand>
    getBuildRunCommands(const std::vector<UnitTest> &testsToLaunch, bool withCoverage) = 0;

    [[nodiscard]] virtual std::vector<ShellExecTask>
    getCoverageCommands(const std::vector<UnitTest> &testsToLaunch) = 0;

    [[nodiscard]] virtual Coverage::CoverageMap getCoverageInfo() const = 0;

    /**
     * Get coverage rate in percents for all processed files.
     * Coverage rates for gcov are more accurate because the
     * calculation ignores UTBot wrappers that boost project rates.
     * @return json with coverage rates. llvm-cov returns line, function
     * and region cover percentages, gcov calculates line coverage only.
     */
    [[nodiscard]] virtual nlohmann::json getTotals() const = 0;

    virtual void cleanCoverage() const = 0;

    virtual ~CoverageTool() = default;
};

std::unique_ptr<CoverageTool> getCoverageTool(const std::string &compileCommandsJsonPath,
                                              utbot::ProjectContext projectContext,
                                              const ProgressWriter *progressWriter);

#endif // UNITTESTBOT_COVERAGETOOL_H
