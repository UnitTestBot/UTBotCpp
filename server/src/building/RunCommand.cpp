#include "RunCommand.h"

#include "exceptions/UnImplementedException.h"
#include "utils/CompilationUtils.h"
#include "utils/StringUtils.h"

namespace utbot {
    RunCommand::RunCommand(std::vector<std::string> commandLine, fs::path directory, bool shouldChangeDirectory)
            : BaseCommand(std::move(commandLine), std::move(directory), shouldChangeDirectory) {
    }

    std::string RunCommand::toStringWithChangingDirectoryToNew(const fs::path &targetDirectory) const {
        std::string baseCommand = toString();
        return StringUtils::stringFormat(R"(cd "%s" && %s)", targetDirectory, baseCommand);
    }

    RunCommand
    RunCommand::forceRemoveFile(const fs::path &path, fs::path const &workingDir, bool shouldChangeDirectory) {
        return {{"rm", "-f", path}, workingDir, shouldChangeDirectory};
    }

    RunCommand
    RunCommand::copyFile(const fs::path &from, const fs::path &to, fs::path const &workingDir) {
        return {{"cp", from, to}, workingDir};
    }

    bool RunCommand::isArchiveCommand() const {
        return false;
    }
}
