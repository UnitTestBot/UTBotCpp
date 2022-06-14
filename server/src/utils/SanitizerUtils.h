#ifndef UNITTESTBOT_SANITIZERUTILS_H
#define UNITTESTBOT_SANITIZERUTILS_H

#include "CompilationUtils.h"

#include "utils/path/FileSystemPath.h"
#include <string>
#include <vector>

namespace SanitizerUtils {
    extern const std::vector<std::string> CLANG_SANITIZER_CHECKS;
    extern const std::string CLANG_SANITIZER_CHECKS_FLAG;

    extern const std::string UBSAN_OPTIONS_NAME;
    extern const std::string UBSAN_OPTIONS_VALUE;
    extern const std::string ASAN_OPTIONS_NAME;
    extern const std::string ASAN_OPTIONS_VALUE;

    std::vector<std::string> getSanitizeCompileFlags(CompilationUtils::CompilerName const &compilerName);
    std::string getSanitizeLinkFlags(CompilationUtils::CompilerName const &compilerName);
};


#endif // UNITTESTBOT_SANITIZERUTILS_H
