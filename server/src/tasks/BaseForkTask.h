/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_BASEFORKTASK_H
#define UNITTESTBOT_BASEFORKTASK_H

#include "utils/LogUtils.h"
#include "utils/ExecutionResult.h"

#include <protobuf/testgen.grpc.pb.h>
#include <run_klee/run_klee.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

class BaseForkTask {
public:
    BaseForkTask() = delete;
    virtual ExecUtils::ExecutionResult run();
    /**
     * @brief Sets the output file path. This path will be used for
     * processes communication. The file is deleted on exit code 0
     * and kept if an error happens.
     * @param path - the path of output file.
     */
    void setLogFilePath(fs::path path);
    /**
     * @brief Disables deletion of the output file in all cases.
     * @param retain - the boolean which indicated should output file
     * be retained in all cases.
     */
    void setRetainOutputFile(bool retain);
    /**
     * @brief Checks if the task was interrupted via its exit code.
     * @param exitCode - the task exit code.
     */
    static bool wasInterrupted(int exitCode);
protected:
    explicit BaseForkTask(std::string processName,
                          const std::optional<std::chrono::seconds> &timeout,
                          fs::path logFilePath,
                          std::vector<int> shutDownSignals,
                          bool redirectStderr,
                          bool ignoreErrors);
    virtual ~BaseForkTask() = default;
    /*
     * Actions that triggers on child process events.
     * You can use those for logging.
     */
    /**
     * @brief Triggers on timeout of the child process.
     */
    virtual void timeoutMessage() const;
    /**
     * @brief Triggers when child process is waited for.
     */
    virtual void waitMessage() const;
    /**
     * @brief Triggers when an error on opening or using
     * child process output file happens.
     */
    virtual void logFailMessage() const;
    /**
     * @brief Triggers when the child process is stopped.
     */
    virtual void stopMessage(int status) const;
    /**
     * @brief Triggers when the child process is killed.
     */
    virtual void killMessage(int status) const;
    /**
     * @brief Triggers when the output of the child process
     * is successfully redirected to log file.
     */
    virtual void redirectMessage() const;
    /**
     * @brief Triggers on child process creation.
     */
    virtual void initMessage() const;

    /**
     * @brief Triggers after sending signal to child process.
     */
    virtual void waitAfterSignal(int signalId) const = 0;

    /**
     * @brief Reads the output file to std::string.
     */
    virtual std::string collectAndCleanup() = 0;

    /**
     * @brief The function that is invoked in the child process.
     */
    virtual int childProcessJob() = 0;

    /**
     * @brief Redirects child process stdout (and, optionally,
     * stderr) to output file.
     */
    bool redirectOutput();

    /**
     * @brief Log out if the process is running or has ended.
     * @param pid - pid of the process to check.
     */
    static void checkForExist(pid_t pid);

    /**
     * @brief wait on pid and then proceed to kill it on timeout
     * @return status of the finished process.
     */
    int waitForFinishedOrCancelled();

    /**
     * Pid of the child process, used to track its status.
     */
    pid_t pid;
    /**
     * Name of the binary running in the child process,
     * used for logging.
     */
    std::string processName;
    /**
     * Output file path.
     */
    fs::path logFilePath;
    /**
     * Timeout after which the task is automatically cancelled.
     * Cancellation process consecutively sends shutDownSignals
     * to the child process.
     * Set to std::nullopt if you do not want the process to be
     * cancellable.
     */
    const std::optional<std::chrono::seconds> timeout;
    /**
     * A sequence of signals sent to child process on cancellation.
     */
    const std::vector <int> shutDownSignals;
    /**
     * Should the process stderr be redirected to output file.
     */
    bool redirectStderr;
    /**
     * Should UTBot log out nonzero exit codes or not.
     */
    bool ignoreErrors;
    /**
     * Internal value used for setting appropriate exit code.
     */
    bool cancelled = false;
    /**
     * Should output file be retained on exit code 0.
     */
    bool retainOutputFile = false;
    /**
     * Exit codes set by child process to indicate
     * special errors.
     */
    static const int LOG_FAIL_CODE = 8;
    static const int TIMEOUT_CODE = 9;
    static const int SETPGID_FAIL_CODE = 10;

    /**
     * Throws if the watched child process is absent.
     */
    static void throwIfNoSuchProcess();
};


#endif // UNITTESTBOT_BASEFORKTASK_H
