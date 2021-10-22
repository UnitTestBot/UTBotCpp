/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "Server.h"
#include "utils/CLIUtils.h"

#include "loguru.h"

#include <cstdlib>

int main(int argc, char **argv) {
    setenv("GRPC_ENABLE_FORK_SUPPORT", "1", 1);
    llvm::sys::PrintStackTraceOnErrorSignal(argv[0]);
    CLI::App app{"Unit tests auto-generation tool for C projects."};
    std::atexit([]() { std::cout << rang::style::reset; });
    try {
        CLIUtils::parse(argc, argv,app);
    } catch (const CLI::ParseError &e) {
        std::cout << (e.get_exit_code() == 0 ? rang::fg::green : rang::fg::red);
        return app.exit(e);
    }
    return 0;
}