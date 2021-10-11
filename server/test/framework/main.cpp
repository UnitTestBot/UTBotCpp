/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "TestUtils.h"
#include "utils/CLIUtils.h"

#include "loguru.h"

#include <llvm/Support/Signals.h>

//Usage: ./UTBot_UnitTests [--verbosity trace|debug|info|warning|error]
int main(int argc, char **argv) {
    llvm::sys::PrintStackTraceOnErrorSignal(argv[0]);

    ::testing::InitGoogleTest(&argc, argv);

    auto ctx = std::make_unique<ServerContext>();
    ServerUtils::setThreadOptions(ctx.get(), true);

    CLIUtils::setupLogger(argc, argv, false);

    fs::path logFilePath = Paths::getBaseLogDir();
    fs::path allLogPath = logFilePath / "everything.log";
    fs::path latestLogPath = logFilePath / "latest_readable.log";
    loguru::add_file(allLogPath.c_str(), nullptr, loguru::Append, loguru::Verbosity_MAX);
    loguru::add_file(latestLogPath.c_str(), nullptr, loguru::Truncate, loguru::Verbosity_INFO);

    auto clang = CompilationUtils::CompilerName::CLANG;
    auto gcc = CompilationUtils::CompilerName::GCC;
    auto cmake = testUtils::BuildCommandsTool::CMAKE_BUILD_COMMANDS_TOOL;
    auto bear = testUtils::BuildCommandsTool::BEAR_BUILD_COMMANDS_TOOL;

    try {
        testUtils::tryExecGetBuildCommands(testUtils::getRelativeTestSuitePath("server"), gcc);

        testUtils::tryExecGetBuildCommands(testUtils::getRelativeTestSuitePath("server"), clang);

        testUtils::tryExecGetBuildCommands(testUtils::getRelativeTestSuitePath("syntax"));

        testUtils::tryExecGetBuildCommands(testUtils::getRelativeTestSuitePath("coverage"), clang, cmake, true);

        testUtils::tryExecGetBuildCommands(testUtils::getRelativeTestSuitePath("coverage"), gcc, cmake, true);

        testUtils::tryExecGetBuildCommands(testUtils::getRelativeTestSuitePath("stub"));

        testUtils::tryExecGetBuildCommands(testUtils::getRelativeTestSuitePath("regression"));

        testUtils::tryExecGetBuildCommands(testUtils::getRelativeTestSuitePath("char"), clang, cmake, true);

        testUtils::tryExecGetBuildCommands(testUtils::getRelativeTestSuitePath("char"), gcc, cmake, true);

        testUtils::tryExecGetBuildCommands(testUtils::getRelativeTestSuitePath("cli"));

        for (auto const &subproject : { "executable", "static_library", "shared_library", "timeout" }) {
            for (auto const &compiler : { clang, gcc }) {
                testUtils::tryExecGetBuildCommands(testUtils::getRelativeTestSuitePath("run") / subproject, compiler);
            }
        }

        testUtils::tryExecGetBuildCommands(testUtils::getRelativeTestSuitePath("halt"), clang);

        testUtils::tryExecGetBuildCommands(testUtils::getRelativeTestSuitePath("halt"), gcc);

        testUtils::tryExecGetBuildCommands(testUtils::getRelativeTestSuitePath("datacom"), clang);

        testUtils::tryExecGetBuildCommands(testUtils::getRelativeTestSuitePath("datacom"), gcc);

        testUtils::tryExecGetBuildCommands(testUtils::getRelativeTestSuitePath("targets"), clang);

        testUtils::tryExecGetBuildCommands(testUtils::getRelativeTestSuitePath("object-file"), clang, testUtils::MAKE_BUILD_COMMANDS_TOOL);
    } catch (std::runtime_error const &e) {
        return 1;
    }

    return RUN_ALL_TESTS();
}