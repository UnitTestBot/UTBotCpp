#ifndef UNITTESTBOT_RUNCOMMAND_H
#define UNITTESTBOT_RUNCOMMAND_H

#include "BaseCommand.h"

#include <string>
#include <vector>

namespace utbot {
    class RunCommand : public BaseCommand {
    public:
        RunCommand(std::vector<std::string> commandLine, fs::path directory, bool shouldChangeDirectory = false);
        fs::path getOutput() const override;

        std::string toStringWithChangingDirectoryToNew(const fs::path &targetDirectory) const override;

        static RunCommand forceRemoveFile(fs::path const &path, fs::path const &workingDir, bool shouldChangeDirectory = false);

        static RunCommand
        copyFile(fs::path const &from, const fs::path &to, fs::path const &workingDir);

        bool isArchiveCommand() const override;
    };
}


#endif // UNITTESTBOT_RUNCOMMAND_H
