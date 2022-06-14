#ifndef UNITTESTBOT_MAKEFILEUTIL_H
#define UNITTESTBOT_MAKEFILEUTIL_H

#include "ExecUtils.h"
#include "ProjectContext.h"

#include "tasks/ShellExecTask.h"

#include "utils/path/FileSystemPath.h"
#include <string>

namespace MakefileUtils {
    class MakefileCommand {
        fs::path makefile;
        std::string target;
        std::string projectName;
        ShellExecTask::ExecutionParameters runCommand, printCommand, echoCommand;
        fs::path logFile;
        mutable ShellExecTask::ExecutionParameters const * failedCommand = nullptr;
    public:

        MakefileCommand() = default;

        MakefileCommand(const utbot::ProjectContext &projectContext,
                        fs::path makefile,
                        std::string target,
                        const std::string &gtestFlags,
                        std::vector<std::string> env);

        [[nodiscard]] ExecUtils::ExecutionResult run(const fs::path &buildPath = "",
                                                 bool redirectStderr = true,
                                                 bool ignoreErrors = false,
                                                 const std::optional<std::chrono::seconds> &timeout = std::nullopt) const;

        [[nodiscard]] std::string getFailedCommand() const;
    };

    MakefileCommand makefileCommand(utbot::ProjectContext const &projectContext,
                                    const fs::path &makefile,
                                    const std::string &target,
                                    const std::string &gtestFlags = "",
                                    const std::vector <std::string> &env = {});

    std::vector<std::string> getMakeCommand(std::string makefile, std::string target, bool nested);

    std::string threadFlag();
}

#endif //UNITTESTBOT_MAKEFILEUTIL_H
