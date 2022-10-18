#include "CoverageTool.h"

#include "GcovCoverageTool.h"
#include "LlvmCoverageTool.h"
#include "exceptions/CoverageGenerationException.h"
#include "utils/CompilationUtils.h"
#include "utils/StringUtils.h"

using namespace CompilationUtils;

CoverageTool::CoverageTool(utbot::ProjectContext projectContext, ProgressWriter const *progressWriter) :
        projectContext(std::move(projectContext)), progressWriter(progressWriter) {
}

std::unique_ptr<CoverageTool> getCoverageTool(const std::string &compileCommandsJsonPath,
                                              utbot::ProjectContext projectContext,
                                              ProgressWriter const *progressWriter) {
    auto compilationDatabase = CompilationUtils::getCompilationDatabase(compileCommandsJsonPath);
    fs::path compilerPath = compilationDatabase->getBuildCompilerPath();
    CompilerName compilerName = CompilationUtils::getCompilerName(compilerPath);
    switch (compilerName) {
    case CompilerName::GCC:
    case CompilerName::GXX:
        return std::make_unique<GcovCoverageTool>(projectContext, progressWriter);
    case CompilerName::CLANG:
    case CompilerName::CLANGXX:
        return std::make_unique<LlvmCoverageTool>(projectContext, progressWriter);
    default:
        throw CoverageGenerationException("Coverage tool for your compiler is not implemented");
    }
}

std::string CoverageTool::getGTestFlags(const UnitTest &unitTest) const {
    std::string gtestFilterFlag = StringUtils::stringFormat("\"--gtest_filter=*.%s\"", unitTest.testname);
    std::string gtestOutputFlag = StringUtils::stringFormat("\"--gtest_output=json:%s\"",
                                                            Paths::getGTestResultsJsonPath(projectContext));
    std::vector<std::string> gtestFlagsList = { gtestFilterFlag, gtestOutputFlag };
    return StringUtils::joinWith(gtestFlagsList, " ");
}
