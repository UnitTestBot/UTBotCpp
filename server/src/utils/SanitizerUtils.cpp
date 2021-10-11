/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "SanitizerUtils.h"

#include "CompilationUtils.h"
#include "StringUtils.h"

namespace SanitizerUtils {
    // https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html#available-checks
    const vector<string> CLANG_SANITIZER_CHECKS = {
        "alignment",
        // "bool", provides unnecessary tests
        "builtin",
        "bounds",
        "enum",
        "float-cast-overflow",
        // "function", // C++ only, not implemented in KLEE
        "float-divide-by-zero",
        "implicit-unsigned-integer-truncation",
        "implicit-signed-integer-truncation",
        "implicit-integer-sign-change",
        "integer-divide-by-zero",
        "nonnull-attribute",
        "null",
        "nullability-arg",
        "nullability-assign",
        "nullability-return",
//        "object-size", the object size sanitizer has no effect at -O0
        "pointer-overflow",
        "return", // C++ only, doesn't affect C code
        "returns-nonnull-attribute",
        "shift",
        // "unsigned-shift-base", is supported since clang-12
        // "signed-integer-overflow", too verbose
        "unreachable",
        // "unsigned-integer-overflow", too verbose
        "vla-bound",
        // "vptr" // C++ only, not implemented in KLEE
    };

    // https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html
    const vector<string> GCC_SANITIZER_CHECKS = {
        "shift",
        "shift-exponent",
        "integer-divide-by-zero",
        "unreachable",
        "vla-bound",
        "null",
        "return", // C++ only, doesn't affect C code
//        "signed-integer-overflow", too verbose
        "bounds",
        "bounds-strict",
        "alignment",
        "object-size",
        "float-divide-by-zero",
        "float-cast-overflow",
        "nonnull-attribute",
        "returns-nonnull-attribute",
        // "bool", provides unnecessary tests
        "enum",
        // "vptr", // C++ only, not implemented in KLEE
        "pointer-overflow",
        "builtin"
    };

    const string CLANG_SANITIZER_CHECKS_FLAG =
        "-fsanitize=" + StringUtils::joinWith(CLANG_SANITIZER_CHECKS, ",");
    static const string GCC_SANITIZER_CHECKS_FLAG =
        "-fsanitize=" + StringUtils::joinWith(GCC_SANITIZER_CHECKS, ",");
    static const string ADDRESS_SANITIZER_FLAG = "-fsanitize=address";

    static const string NO_SANITIZE_RECOVER = "-fno-sanitize-recover=all";

    const std::string UBSAN_OPTIONS_NAME = "UBSAN_OPTIONS";
    const std::string UBSAN_OPTIONS_VALUE = "print_stacktrace=1,report_error_type=1";
    const std::string ASAN_OPTIONS_NAME = "ASAN_OPTIONS";
    const std::string ASAN_OPTIONS_VALUE = "debug=1,detect_odr_violation=1,"
                                           "detect_stack_use_after_return=1,detect_leaks=0";

    std::vector<string>
    getSanitizeCompileFlags(CompilationUtils::CompilerName const &compilerName) {
        switch (compilerName) {
        case CompilationUtils::CompilerName::GCC:
        case CompilationUtils::CompilerName::GXX:
            return { GCC_SANITIZER_CHECKS_FLAG, ADDRESS_SANITIZER_FLAG, NO_SANITIZE_RECOVER };
        case CompilationUtils::CompilerName::CLANG:
        case CompilationUtils::CompilerName::CLANGXX:
            return { CLANG_SANITIZER_CHECKS_FLAG, ADDRESS_SANITIZER_FLAG, NO_SANITIZE_RECOVER };
        default:
            // should never happen
            return {};
        }
    }


    string getSanitizeLinkFlags(CompilationUtils::CompilerName const &compilerName) {
        switch (compilerName) {
        case CompilationUtils::CompilerName::GCC:
        case CompilationUtils::CompilerName::GXX:
            return StringUtils::joinWith({ GCC_SANITIZER_CHECKS_FLAG, ADDRESS_SANITIZER_FLAG },
                                         " ");
        case CompilationUtils::CompilerName::CLANG:
        case CompilationUtils::CompilerName::CLANGXX:
            return StringUtils::joinWith({ CLANG_SANITIZER_CHECKS_FLAG, ADDRESS_SANITIZER_FLAG },
                                         " ");
        default:
            // should never happen
            return "";
        }
    }
};
