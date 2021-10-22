/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "MakefileUtils.h"

#include "CLIUtils.h"
#include "ExecUtils.h"
#include "LogUtils.h"
#include "Paths.h"
#include "StringUtils.h"
#include "commands/Commands.h"
#include "environment/EnvironmentPaths.h"
#include "exceptions/ExecutionProcessException.h"

#include "loguru.hpp"

#include <thread>

namespace MakefileUtils {
    using std::string;

    MakefileCommand::MakefileCommand(const utbot::ProjectContext &projectContext,
                                     fs::path makefile,
                                     string target,
                                     const std::string &gtestFlags,
                                     std::vector<std::string> env)
        : makefile(std::move(makefile)), target(std::move(target)),
          projectName(projectContext.projectName) {
        this->makefile = this->makefile.lexically_normal();
        fs::path logDir = Paths::getLogDir(projectContext.projectName);
        logFile = logDir / "makefile.log";
        fs::create_directories(logDir);
        std::vector<string> argv = std::move(env);
        argv.emplace_back(string("GTEST_FLAGS=\"") + gtestFlags + "\"");
        argv.emplace_back(Paths::getMake());
        argv.emplace_back(threadFlag());
        argv.emplace_back("-s");
        argv.emplace_back("-f");
        argv.emplace_back(this->makefile);
        argv.emplace_back(this->target);
        runCommand = ShellExecTask::ExecutionParameters("env", argv);
        printCommand = ShellExecTask::ExecutionParameters("env", argv);
        printCommand.argv.emplace_back("-n");
        echoCommand = ShellExecTask::ExecutionParameters("echo");
    }

    ExecUtils::ExecutionResult
    MakefileCommand::run(const fs::path &buildPath,
                         bool redirectStderr,
                         bool ignoreErrors,
                         const std::optional<std::chrono::seconds> &timeout) const {
        auto print = ShellExecTask::runShellCommandTaskToFile(printCommand, logFile, buildPath);
        if (print.status != 0) {
            failedCommand = &printCommand;
            return print;
        }
        // This writes \n to logFile to separate runs.
        auto echo = ShellExecTask::runShellCommandTaskToFile(echoCommand, logFile, buildPath);
        if (echo.status != 0) {
            failedCommand = &echoCommand;
            return echo;
        }
        auto exec = ShellExecTask::runShellCommandTask(
            runCommand, buildPath, projectName, redirectStderr, false, ignoreErrors, timeout);
        if (exec.status != 0) {
            failedCommand = &runCommand;
        }
        return exec;
    }

    string MakefileCommand::getFailedCommand() const {
        if (failedCommand) {
            return failedCommand->toString();
        } else {
            LOG_S(ERROR) << "No command failed";
            return "";
        }
    }


    MakefileCommand makefileCommand(utbot::ProjectContext const &projectContext,
                                    const fs::path &makefile,
                                    const std::string &target,
                                    const std::string &gtestFlags,
                                    const std::vector<std::string> &env) {
        return MakefileCommand(projectContext, makefile, target, gtestFlags, env);
    }

    string threadFlag() {
        if (Commands::threadsPerUser != 0) {
            return "-j" + std::to_string(Commands::threadsPerUser);
        }
        unsigned int threads = std::thread::hardware_concurrency();
        if (threads == 0) {
            return "";
        } else {
            return "-j" + std::to_string(threads);
        }
    }
}