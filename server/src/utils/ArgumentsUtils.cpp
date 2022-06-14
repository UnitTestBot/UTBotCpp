#include "ArgumentsUtils.h"

#include "CollectionUtils.h"
#include "CompilationUtils.h"
#include "StringUtils.h"
#include "environment/EnvironmentPaths.h"
#include "exceptions/UnImplementedException.h"

#include "loguru.h"

using namespace CompilationUtils;

namespace CompilationUtils {
    fs::path toCppCompiler(const fs::path &compilerPath) {
        auto compiler = fs::path(compilerPath).filename().string();
        switch (getCompilerName(compiler)) {
        case CompilerName::GCC: {
            std::string result = compilerPath.string();
            StringUtils::replaceLast(result, "gcc", "g++");
            if (fs::exists(result)) {
                return result;
            }
            fs::path directory = fs::path(result).parent_path();
            fs::path candidate = directory / "g++";
            if (fs::exists(candidate)) {
                return candidate;
            }
            return Paths::getGpp();
        }
        case CompilerName::GXX:
            return compilerPath;
        case CompilerName::CLANG: {
            std::string result = compilerPath.string();
            StringUtils::replaceLast(result, "clang", "clang++");
            if (fs::exists(result)) {
                return result;
            }
            fs::path directory = fs::path(result).parent_path();
            fs::path candidate = directory / "clang++";
            if (fs::exists(candidate)) {
                return candidate;
            }
            return Paths::getUTBotClangPP();
        }
        case CompilerName::CLANGXX:
            return compilerPath;
        default:
            LOG_S(WARNING) << StringUtils::stringFormat(
                "C++ version of compiler for %s is unknown, using original compiler", compiler);
            return compilerPath;
        }
    }

    fs::path toCppLinker(const fs::path &linkerPath) {
        auto linker = fs::path(linkerPath).filename().string();
        if (StringUtils::contains(linker, "clang++")) {
            return linkerPath;
        }
        if (StringUtils::contains(linker, "clang")) {
            auto result = linkerPath.string();
            StringUtils::replaceLast(result, "clang", "clang++");
            return result;
        }
        if (StringUtils::contains(linker, "g++")) {
            return linker;
        }
        if (StringUtils::endsWith(linker, "gcc")) {
            auto result = linkerPath.string();
            StringUtils::replaceLast(result, "gcc", "g++");
            return result;
        }
        LOG_S(WARNING) << StringUtils::stringFormat(
            "C++ version of linker for %s is unknown, using original linker", linker);
        return linker;
    }

    std::vector<std::string> getCoverageCompileFlags(const CompilerName &compilerName) {
        switch (compilerName) {
        case CompilerName::GCC:
        case CompilerName::GXX:
            return { "--coverage" };
        case CompilerName::CLANG:
        case CompilerName::CLANGXX:
            return { "-fprofile-instr-generate", "-fcoverage-mapping" };
        default:
            break;
        }
        LOG_S(WARNING) << StringUtils::stringFormat(
            "Coverage flags for compiler: %s are unknown, using empty list of flags",
            to_string(compilerName));
        return {};
    }

    std::vector<std::string> getCoverageLinkFlags(const CompilerName &compilerName) {
        switch (compilerName) {
        case CompilerName::GCC:
        case CompilerName::GXX:
            return { "-lgcov", "--coverage" };
        case CompilerName::CLANG:
        case CompilerName::CLANGXX:
            return { "-fprofile-instr-generate" };
        case CompilerName::UNKNOWN:
            break;
        }
        LOG_S(WARNING) << StringUtils::stringFormat(
            "Coverage flags for compiler: %s are unknown, using empty list of flags.",
            to_string(compilerName));
        return {};
    }

    std::string getPthreadFlag(const CompilerName &compilerName) {
        switch (compilerName) {
        case CompilerName::GCC:
        case CompilerName::GXX:
            return "-pthread";
        case CompilerName::CLANG:
        case CompilerName::CLANGXX:
            return "-lpthread";
        case CompilerName::UNKNOWN:
            return "";
        }
    }
}
