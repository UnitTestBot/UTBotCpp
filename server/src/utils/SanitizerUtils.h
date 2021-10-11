/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_SANITIZERUTILS_H
#define UNITTESTBOT_SANITIZERUTILS_H

#include "CompilationUtils.h"

#include "utils/path/FileSystemPath.h"
#include <string>
#include <vector>

namespace SanitizerUtils {
    using std::vector;
    using std::string;

    extern const std::vector<std::string> CLANG_SANITIZER_CHECKS;
    extern const std::string CLANG_SANITIZER_CHECKS_FLAG;

    extern const std::string UBSAN_OPTIONS_NAME;
    extern const std::string UBSAN_OPTIONS_VALUE;
    extern const std::string ASAN_OPTIONS_NAME;
    extern const std::string ASAN_OPTIONS_VALUE;

    std::vector<string> getSanitizeCompileFlags(CompilationUtils::CompilerName const &compilerName);
    string getSanitizeLinkFlags(CompilationUtils::CompilerName const &compilerName);
};


#endif // UNITTESTBOT_SANITIZERUTILS_H
