#ifndef UNITTESTBOT_EXECUTILS_H
#define UNITTESTBOT_EXECUTILS_H

#include "CollectionUtils.h"
#include "exceptions/CancellationException.h"
#include "streams/IStreamWriter.h"
#include "streams/ProgressWriter.h"
#include "tasks/ShellExecTask.h"
#include "ExecutionResult.h"

#include <grpcpp/grpcpp.h>

#include "utils/path/FileSystemPath.h"

/**
 * @brief Contains useful util functions
 */
namespace ExecUtils {
    using grpc::ServerContext;

    /**
     * @brief Executes command in a child process. `popen` is used.
     * @param command Unix command which must be executed.
     * @param fromDir If passed, execute command from this directory.
     * @param redirectStderr If true, redirects stderr to stdout.
     * @return Pair of stdout and returned code.
     */
     ExecutionResult exec(const std::string &command,
                         const std::string &fromDir = "",
                         const std::string &projectName = "",
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

    void toCArgumentsPtr(std::vector<std::string> &argv,
                         std::vector<std::string> &envp,
                         std::vector<char *> &cargv,
                         std::vector<char *> &cenvp,
                         bool appendNull);

    std::vector<std::string> environAsVector();
}

#endif // UNITTESTBOT_EXECUTILS_H
