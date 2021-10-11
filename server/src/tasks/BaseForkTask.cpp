/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "BaseForkTask.h"
#include "RequestEnvironment.h"
#include "exceptions/BaseException.h"
#include "utils/ExecUtils.h"

#include <grpc/impl/codegen/fork.h>
#include <thread>
#include <utility>

class NoSuchProcessException : public std::exception {
    std::string errnoMessage;

public:
    explicit NoSuchProcessException(std::string errnoMessage)
            : errnoMessage(std::move(errnoMessage)) {
    }

    [[nodiscard]] const char *what() const

    noexcept override{
            return errnoMessage.c_str();
    }
};

BaseForkTask::BaseForkTask(std::string processName,
                           const std::optional<std::chrono::seconds> &timeout,
                           fs::path logFilePath,
                           std::vector<int> shutDownSignals,
                           bool redirectStderr,
                           bool ignoreErrors)
    : processName(std::move(processName)), timeout(timeout), logFilePath(std::move(logFilePath)),
          shutDownSignals(std::move(shutDownSignals)), redirectStderr(redirectStderr), ignoreErrors(ignoreErrors) {
}

ExecUtils::ExecutionResult BaseForkTask::run() {
    grpc_prefork();
    switch (pid = fork()) {
        case -1: {
            auto message = processName + " fork failed.";
            LOG_S(ERROR) << message << LogUtils::errnoMessage();
            throw BaseException(message);
        }
        case 0: {
            grpc_postfork_child();
            // This is child process
            if (!redirectOutput()) {
                exit(LOG_FAIL_CODE);
            }
            int exitCode = childProcessJob();
            exit(exitCode);
        }
        default: {
            grpc_postfork_parent();
            // This is parent process
            LOG_S(DEBUG) << "Running " << processName << " out of process from pid: " << getpid();
            initMessage();
            int status = waitForFinishedOrCancelled();
            string output = collectAndCleanup();
            if (cancelled) {
                status = TIMEOUT_CODE;
            }
            if (!ignoreErrors && status && status != TIMEOUT_CODE) {
                LOG_S(ERROR) << "Exit status: " << status;
                LOG_S(ERROR) << "See details in " << logFilePath;
            }
            LOG_IF_S(DEBUG, status == 0) << "Exit status: 0";
            if (status == 0 && !retainOutputFile) {
                fs::remove(logFilePath);
                return {output, status, std::nullopt};
            } else {
                return {output, status, logFilePath};
            }
        }
    }
}

bool BaseForkTask::wasInterrupted(int exitCode) {
    return (exitCode == TIMEOUT_CODE);
}

void BaseForkTask::timeoutMessage() const {
}

void BaseForkTask::waitMessage() const {
}

void BaseForkTask::logFailMessage() const {
}

void BaseForkTask::stopMessage(int status) const {
}

void BaseForkTask::killMessage(int status) const {
}

void BaseForkTask::redirectMessage() const {
}

void BaseForkTask::initMessage() const {
}


bool BaseForkTask::redirectOutput() {
    redirectMessage();
    fs::create_directories(logFilePath.parent_path());
    int fd = open(logFilePath.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    bool ok = true;
    if (fd == -1) {
        ok = false;
        LOG_S(ERROR) << "Failed to create temporary logging file for " << processName << ": "
                     << LogUtils::errnoMessage() << "(" << logFilePath << ")";
    }
    if (dup2(fd, 1) == -1 || (redirectStderr ? (dup2(fd, 2) == -1) : false)) {
        ok = false;
        LOG_S(ERROR) << "Failed to create descriptors for child process: "
                     << LogUtils::errnoMessage();
    }
    close(fd);
    return ok;
}

void BaseForkTask::checkForExist(pid_t pid) {
    int existStatus = kill(pid, 0);
    if (existStatus == -1 && errno == ESRCH) {
        LOG_S(DEBUG) << "Child process was killed";
    } else {
        LOG_S(WARNING) << "Child process was not killed";
    }
}


void BaseForkTask::throwIfNoSuchProcess() {
    if (errno == ESRCH) {
        throw NoSuchProcessException(strerror(errno));
    }
}

/**
 * @brief wait on pid and then proceed to kill it on timeout
 * @return status of the finished process.
 */
int BaseForkTask::waitForFinishedOrCancelled() {
    try {
        int status = 0;
        int signalId = 0;
        int spent_ms = 0;
        bool sendSignals = false;
        auto start = std::chrono::steady_clock::now();
        bool ok = false;
        while (true) {
            if (timeout.has_value()) {
                auto now = std::chrono::steady_clock::now();
                if ((now - start) > timeout.value()) {
                    timeoutMessage();
                    sendSignals = true;
                }
            }
            if (RequestEnvironment::isCancelled()) {
                LOG_S(DEBUG) << "Stopping " << processName << " as cancellation was received";
                sendSignals = true;
            }
            if (sendSignals) {
                if (signalId == shutDownSignals.size()) {
                    LOG_S(WARNING) << "Process was not killed";
                    break;
                }
                int signal = shutDownSignals[signalId];
                throwIfNoSuchProcess();
                LOG_S(DEBUG) << "Sending signal to " << processName << ": " << signal;
                int killStatus = kill(pid, signal);
                if (killStatus == -1) {
                    LOG_S(DEBUG) << "Failed to send signal to " << processName << ": "
                                 << LogUtils::errnoMessage();
                    continue; // Trying next level of shutdown
                }
                LOG_S(DEBUG) << "Successfully sent signal to " << processName;
                waitAfterSignal(signalId);
                signalId++;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            pid_t result = waitpid(pid, &status, WNOHANG | WUNTRACED);
            if (result == 0) {
                spent_ms++;
                if (spent_ms == 1000) {
                    spent_ms = 0;
                    waitMessage();
                }
            } else if (result == pid && WIFEXITED(status)) {
                int exitStatus = WEXITSTATUS(status);
                if (exitStatus == LOG_FAIL_CODE) {
                    logFailMessage();
                }
                cancelled &= sendSignals;
                return exitStatus;
            } else if (WIFSIGNALED(status)) {
                killMessage(status);
                cancelled &= sendSignals;
                return status;
            } else if (WIFSTOPPED(status)) {
                stopMessage(status);
                sendSignals = true;
                continue;
            } else {
                LOG_S(WARNING) << "Received undefined status: " << status << ". Ignoring that.";
                continue;
            }
        }
        checkForExist(pid);
        return -1;
    } catch (NoSuchProcessException const &e) {
        return -1;
    }
}

void BaseForkTask::setLogFilePath(fs::path path) {
    logFilePath = std::move(path);
}

void BaseForkTask::setRetainOutputFile(bool retain) {
    retainOutputFile = retain;
}

