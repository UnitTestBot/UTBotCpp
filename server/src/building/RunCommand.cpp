#include "RunCommand.h"

#include "exceptions/UnImplementedException.h"
#include "utils/CompilationUtils.h"
#include "utils/StringUtils.h"

namespace utbot {
    RunCommand::RunCommand(std::vector<std::string> commandLine, fs::path directory)
        : BaseCommand(std::move(commandLine), std::move(directory)) {
    }
    fs::path RunCommand::getOutput() const {
        throw UnImplementedException("RunTestsCommand doesn't have output by default");
    }
    std::string RunCommand::toStringWithChangingDirectory() const {
        std::string baseCommand = toString();
        return StringUtils::stringFormat(R"(cd "%s" && %s)", directory, baseCommand);
    }
    RunCommand RunCommand::forceRemoveFile(const fs::path &path, fs::path const &workingDir) {
        return { { "rm", "-f", path }, workingDir };
    }

    RunCommand
    RunCommand::copyFile(const fs::path &from, const fs::path &to, fs::path const &workingDir) {
        return { { "cp", from, to }, workingDir };
    }

    bool RunCommand::isArchiveCommand() const {
        return false;
    }
}
