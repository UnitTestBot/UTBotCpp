/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_RUNKLEETASK_H
#define UNITTESTBOT_RUNKLEETASK_H
#include "BaseForkTask.h"
#include "Paths.h"


/**
 * Class that performs a fork, calls run_klee(argc, argv, environ)
 * in chind process, writes KLEE directory and dumps KLEE output
 * to DEBUG log. ::run() always returns empty output.
 */
class RunKleeTask : public BaseForkTask {
public:
    /**
     * @brief gets KLEE task to be run of process for .ktest generation.
     * @param argc - the number of arguments sent to KLEE.
     * @param argv - KLEE options for the run.
     * @param timeout - Number of seconds after which the KLEE process
     * is stopped.
     */
    explicit RunKleeTask(int argc,
                         char **argv,
                         const std::optional<std::chrono::seconds> &timeout);

    ExecUtils::ExecutionResult run() override;
private:
    void timeoutMessage() const override;
    void waitMessage() const override;
    void logFailMessage() const override;
    void killMessage(int status) const override;
    void stopMessage(int status) const override;
    void redirectMessage() const override;
    void waitAfterSignal(int signalId) const override;
    int childProcessJob() override;
    std::string collectAndCleanup() override;

    static constexpr std::chrono::milliseconds DUMP_TIMEOUT_MILLISECONDS { 5'000 }; // 5s
    static constexpr std::chrono::milliseconds TIMEOUT_MILLISECONDS{ 100 }; // 100ms

    std::function <int(void)> runKleeLambda;
};


#endif // UNITTESTBOT_RUNKLEETASK_H
