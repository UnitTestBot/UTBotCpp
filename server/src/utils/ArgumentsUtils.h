#ifndef UNITTESTBOT_ARGUMENTSUTILS_H
#define UNITTESTBOT_ARGUMENTSUTILS_H

#include "CompilationUtils.h"
#include "ExecUtils.h"

#include "utils/path/FileSystemPath.h"
#include <string>
#include <vector>

namespace CompilationUtils {
    fs::path toCppCompiler(const fs::path &compilerPath);

    fs::path toCppLinker(const fs::path &linker);

    std::vector<std::string> getCoverageCompileFlags(const CompilerName &compilerName);

    std::vector<std::string> getCoverageLinkFlags(const CompilationUtils::CompilerName &compilerName);

    std::string getPthreadFlag(const CompilerName &compilerName);

}

#endif //UNITTESTBOT_ARGUMENTSUTILS_H
