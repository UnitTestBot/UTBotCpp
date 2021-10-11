/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "RunKleeTask.h"

#include "TimeExecStatistics.h"
#include "utils/ExecUtils.h"

#include <thread>

using namespace std::chrono_literals;

void RunKleeTask::timeoutMessage() const {
    LOG_S(WARNING) << "Time is up (" << timeout->count() << "s). Stop executing.";
}
void RunKleeTask::waitMessage() const {
    LOG_S(MAX) << processName << " is still running";
}
void RunKleeTask::logFailMessage() const {
    LOG_S(DEBUG) << processName
                 << " was not launched due to error in redirecting its output streams";
}
void RunKleeTask::killMessage(int status)const {
    LOG_S(DEBUG) << processName << " was killed by signal: " << WTERMSIG(status);
}
void RunKleeTask::stopMessage(int status) const {
    LOG_S(DEBUG) << processName << " was stopped by signal: " << WSTOPSIG(status);
}
void RunKleeTask::redirectMessage() const {
    LOG_S(DEBUG) << "Redirecting " << processName << " output to file: " << logFilePath;
}

ExecUtils::ExecutionResult RunKleeTask::run() {
    MEASURE_FUNCTION_EXECUTION_TIME
    return BaseForkTask::run();
}

void RunKleeTask::waitAfterSignal(int signalId) const {
    if (signalId == 0) {
        // Dump may take a while, so try and give the process extra time to clean up
        for (int i = 0; i < DUMP_TIMEOUT_MILLISECONDS / TIMEOUT_MILLISECONDS; i++) {
            std::this_thread::sleep_for(TIMEOUT_MILLISECONDS);
            throwIfNoSuchProcess();
        }
    } else {
        std::this_thread::sleep_for(TIMEOUT_MILLISECONDS);
    }
}

int RunKleeTask::childProcessJob() {
    return runKleeLambda();
}

std::string RunKleeTask::collectAndCleanup() {
    /* actually does not return output as it is
     * prettier to use LOG_SCOPE_FUNCTION
    */
    LOG_SCOPE_FUNCTION(DEBUG);
    std::ifstream logFile(logFilePath);
    std::string buf;
    while (std::getline(logFile, buf)) {
        LOG_S(DEBUG) << buf;
    }
    return "";
}

RunKleeTask::RunKleeTask(int argc,
                         char **argv,
                         const std::optional<std::chrono::seconds> &timeout)
    : BaseForkTask("KLEE", timeout, Paths::getKleeTmpLogFilePath(), { SIGTERM, SIGTERM, SIGKILL }, true, true),
      runKleeLambda([=] { return run_klee(argc, argv, environ); }) {
}
