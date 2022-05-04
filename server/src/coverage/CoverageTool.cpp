/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "CoverageTool.h"

#include "GcovCoverageTool.h"
#include "LlvmCoverageTool.h"
#include "exceptions/CoverageGenerationException.h"
#include "utils/CompilationUtils.h"
#include "utils/StringUtils.h"

using namespace CompilationUtils;

CoverageTool::CoverageTool(ProgressWriter const *progressWriter) : progressWriter(progressWriter) {
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

std::string CoverageTool::getTestFilter(const UnitTest &unitTest) const {
    return StringUtils::stringFormat("--gtest_filter=*.%s", unitTest.testname);
}