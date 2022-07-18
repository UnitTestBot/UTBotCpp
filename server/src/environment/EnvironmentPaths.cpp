#include "environment/EnvironmentPaths.h"

namespace Paths {
    fs::path getExecutablePath()
    {
        return fs::canonical("/proc/self/exe");
    }

    static bool isDevEnvironment() {
        return fs::exists(UTBOT_DEV_ROOT_DIR);
    }

    fs::path getUTBotRootDir() {
        if (isDevEnvironment()) {
            return UTBOT_DEV_ROOT_DIR;
        }
        return getExecutablePath().parent_path().parent_path();
    }

    fs::path getUTBotInstallDir() {
        return getUTBotRootDir() / "install";
    }

    fs::path getUTBotDebsInstallDir() {
        if (isDevEnvironment()) {
            return fs::current_path().root_path();
        }
        return getUTBotRootDir() / "debs-install";
    }

    fs::path getPython() {
        return getUTBotDebsInstallDir() / "usr" / "bin" / "python";
    }


    fs::path getBear() {
        return getUTBotRootDir() / "bear" / "bin" / "bear";
    }

    fs::path getCMake() {
        return getUTBotInstallDir() / "bin" / "cmake";
    }

    fs::path getMake() {
        return "make";
    }

    fs::path getUTBotClang() {
        return getUTBotInstallDir() / "bin" / "clang";
    }

    fs::path getUTBotClangPP() {
        return getUTBotInstallDir() / "bin" / "clang++";
    }

    fs::path getGcc() {
        return "gcc";
    }

    fs::path getGpp() {
        return "g++";
    }

    fs::path getLLVMnm() {
        return getUTBotInstallDir() / "bin" / "llvm-nm";
    }

    fs::path getGtestLibPath() {
        return getUTBotRootDir() / "gtest";
    }

    fs::path getAccessPrivateLibPath() {
        return getUTBotRootDir() / "access_private" / "include";
    }

    fs::path getLLVMprofdata() {
        return getUTBotInstallDir() / "bin" / "llvm-profdata";
    }

    fs::path getLLVMcov() {
        return getUTBotInstallDir() / "bin" / "llvm-cov";
    }

    fs::path getLLVMgold() {
        return getUTBotInstallDir() / "lib" / "LLVMgold.so";
    }

    fs::path getAr() {
        return getUTBotDebsInstallDir() / "usr" / "bin" / "ar";
    }

    fs::path getLdGold() {
        return getUTBotDebsInstallDir() / "usr" / "bin" / "ld.gold";
    }

    fs::path getLd() {
        return getUTBotDebsInstallDir() / "usr" / "bin" / "ld";
    }

    fs::path getAsanLibraryPath() {
        return Paths::getUTBotDebsInstallDir() / "usr" / "lib" / "gcc" / "x86_64-linux-gnu" / "9" / "libasan.so";
    }
}
