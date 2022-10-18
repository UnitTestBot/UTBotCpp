#include "MakefileUtils.h"

#include "CLIUtils.h"
#include "ExecUtils.h"
#include "LogUtils.h"
#include "Paths.h"
#include "StringUtils.h"
#include "commands/Commands.h"
#include "environment/EnvironmentPaths.h"
#include "exceptions/ExecutionProcessException.h"

#include "loguru.h"

#include <thread>

namespace MakefileUtils {
    std::vector<std::string> getMakeCommand(std::string makefile, std::string target, bool nested) {
        std::vector<std::string> command;
        if (nested) {
            command.emplace_back("$(MAKE)");
        } else {
            command.emplace_back(Paths::getMake());
            command.emplace_back(threadFlag());
            command.emplace_back("-s");
        }
        command.emplace_back("-f");
        command.emplace_back(makefile);
        command.emplace_back(target);
        return command;
    }

    MakefileCommand::MakefileCommand(const utbot::ProjectContext &projectContext,
                                     fs::path makefile,
                                     std::string target,
                                     const std::string &gtestFlags,
                                     std::vector<std::string> env)
            : makefile(std::move(makefile)), target(std::move(target)),
              projectName(projectContext.projectName) {
        this->makefile = this->makefile.lexically_normal();
        this->makefile = this->makefile.lexically_normal();
        fs::path logDir = Paths::getLogDir(projectContext.projectName);
        logFile = logDir / "makefile.log";
        fs::create_directories(logDir);
        std::vector<std::string> argv = std::move(env);
        argv.emplace_back(std::string("GTEST_FLAGS=") + gtestFlags);
        std::vector<std::string> makeCommand = getMakeCommand(this->makefile, this->target, false);
        argv.insert(argv.begin(), makeCommand.begin(), makeCommand.end());
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

    std::string MakefileCommand::getFailedCommand() const {
        if (failedCommand) {
            return failedCommand->toString();
        } else {
            LOG_S(ERROR) << "No command failed";
            return "";
        }
    }

    std::string threadFlag() {
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
