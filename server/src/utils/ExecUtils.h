/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_EXECUTILS_H
#define UNITTESTBOT_EXECUTILS_H

#include "CollectionUtils.h"
#include "exceptions/CancellationException.h"
#include "streams/IStreamWriter.h"
#include "streams/ProgressWriter.h"
#include "tasks/ShellExecTask.h"

#include <grpcpp/grpcpp.h>

#include "utils/path/FileSystemPath.h"

/**
 * @brief Contains useful util functions
 */
namespace ExecUtils {
    using grpc::ServerContext;
    using std::vector;
    using std::string;

    struct ExecutionResult {
        std::string output;
        int status;
        std::optional<fs::path> outPath;
    };

    /**
     * @brief Executes command in a child process. `popen` is used.
     * @param command Unix command which must be executed.
     * @param fromDir If passed, execute command from this directory.
     * @param redirectStderr If true, redirects stderr to stdout.
     * @return Pair of stdout and returned code.
     */
     ExecutionResult exec(const string &command,
                     const string &fromDir = "",
                     const string &projectName = "",
                     bool redirectStderr = true,
                     bool logOut = false,
                     bool ignoreErrors = false,
                     const std::optional<std::chrono::seconds> &timeout = std::nullopt);

    void throwIfCancelled();

    template <typename Iterable, typename Functor>
    void doWorkWithProgress(Iterable &&iterable,
                            ProgressWriter const *progressWriter,
                            std::string const &message,
                            Functor &&functor) {
        size_t size = iterable.size();
        progressWriter->writeProgress(message);
        for (auto &&it : iterable) {
            throwIfCancelled();
            functor(it);
            progressWriter->writeProgress(message, 100.0 / size);
        }
    }

    void toCArgumentsPtr(vector<std::string> &argv,
                         vector<std::string> &envp,
                         vector<char *> &cargv,
                         vector<char *> &cenvp,
                         bool appendNull);

    std::vector<std::string> environAsVector();
}

#endif // UNITTESTBOT_EXECUTILS_H
