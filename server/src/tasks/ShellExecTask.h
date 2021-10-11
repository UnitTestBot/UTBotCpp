/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_SHELLEXECTASK_H
#define UNITTESTBOT_SHELLEXECTASK_H
#include "BaseForkTask.h"
#include "Paths.h"


namespace utbot {
    class BaseCommand;
}

/**
 * Class that performs a fork and calls execvp(executable, args)
 * in child process for a given command. Can be cancelled/killed.
 * Command output is availible via ::run() and is written in file
 * which path can be obtained by ExecutionResult::outPath.
 */
class ShellExecTask : public BaseForkTask {
public:
    /**
     * Configuration of the job.
     */
    struct ExecutionParameters {
        /**
         * Name of the executable to launch.
         */
        std::string executable{};
        /**
         * Arguments that should be passed to
         * the executable.
         */
        std::vector<std::string> argv{};
        /**
         * Environment variables that the executable
         * should use. Prior to the job launch,
         * variables from **environ are appended to it.
         */
        std::vector<std::string> envp{};
        [[nodiscard]] std::string toString() const;
        ExecutionParameters() = default;
        explicit ExecutionParameters(std::string _executable,
                                     std::vector<std::string> _argv = {},
                                     std::vector<std::string> _argc = {});
    };

    [[nodiscard]] std::string toString() const;

    /**
     * @brief Provides the ShellExecTask instance
     * which can then be launched via ::run().
     * Constructs ExecutionParameters instance from
     * executable and arguments.
     */
    static ShellExecTask
    getShellCommandTask(const std::string &executable,
                     const std::vector<std::string> &arguments,
                     const std::string &fromDir = "",
                     const std::string &projectName = "",
                     bool redirectStderr = true,
                     bool logOut = false,
                     bool ignoreErrors = false,
                     const std::optional<std::chrono::seconds> &timeout = std::nullopt);

    /**
     * @brief Provides the ShellExecTask instance
     * which can then be launched via ::run().
     * Constructs ExecutionParameters instance from
     * executable and arguments. Passes default values
     * to ShellExecTask constructor.
     */
    static ShellExecTask getShellCommandTask(const std::string &executable,
                                             const std::vector<std::string> &arguments,
                                             const std::optional<std::chrono::seconds> &timeout);

    /**
     * @brief Construct ShellExecTask and immediately
     * run it. Output file path is created from projectName.
     */
    static ExecUtils::ExecutionResult
    runShellCommandTask(const ExecutionParameters &params,
                     const std::string &fromDir = "",
                     const std::string &projectName = "",
                     bool redirectStderr = true,
                     bool logOut = false,
                     bool ignoreErrors = false,
                     const std::optional<std::chrono::seconds> &timeout = std::nullopt);

    /**
     * @brief Launches string as a /bin/sh command. Can execute call chains.
     * Despite this having the most friendly API, it should not be used in
     * core, only in unittests as this is not cancellable.
     */
    static ExecUtils::ExecutionResult
    runPlainShellCommand(const std::string &command,
                     const std::string &fromDir = "",
                     const std::string &projectName = "",
                     bool redirectStderr = true,
                     bool logOut = false,
                     bool ignoreErrors = false,
                     const std::optional<std::chrono::seconds> &timeout = std::nullopt);

    /**
     * @brief Construct ShellExecTask and immediately
     * run it. Output file path is specified explicitly.
     * Equivalent to auto task = getShellExecTask(...);
     * task.setLogFilePath(execLogFile);
     */
    static ExecUtils::ExecutionResult
    runShellCommandTaskToFile(const ExecutionParameters &params,
                           const std::string &execLogFile,
                           const std::string &fromDir = "",
                           bool redirectStderr = true,
                           bool logOut = false,
                           bool ignoreErrors = false,
                           const std::optional<std::chrono::seconds> &timeout = std::nullopt);
    /**
     * @brief Construct ShellExecTask from utbot::BaseCommand
     * instance and immediately run it.
     */
    static ExecUtils::ExecutionResult executeUtbotCommand(const utbot::BaseCommand &command,
                                                          const std::string &fromDir,
                                                          const std::string& projectName);

private:
    /**
     * @param _params Executable configuration to be run.
     * @param fromDir If passed, execute command from this directory.
     * @param execLogPath Path to output file which is written by child process.
     * @param redirectStderr If true, redirects stderr to stdout.
     * @param logOut If true, writes command output to DEBUG log.
     * @param ignoreErrors If false, writes nonzero exit code to ERROR log.
     * @param timeout If present, terminate the exec task after specified number of seconds.
     */
    explicit ShellExecTask(ExecutionParameters _params,
                           const std::string &fromDir,     /* = ""*/
                           const std::string &execLogPath,
                           bool redirectStderr,            /* = true*/
                           bool logOut,                    /* = false*/
                           bool ignoreErrors,              /* = false*/
                           const std::optional<std::chrono::seconds> &timeout /* = std::nullopt*/);

    void initMessage() const override;
    void waitAfterSignal(int signalId) const override;

    ExecutionParameters params;
    std::vector <char*> cargv, cenvp;
    fs::path workDir;
    int childProcessJob() override;
    std::string collectAndCleanup() override;
    bool logOut;
};

#endif // UNITTESTBOT_SHELLEXECTASK_H