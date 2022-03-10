/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_RUNCOMMAND_H
#define UNITTESTBOT_RUNCOMMAND_H

#include "BaseCommand.h"

#include <string>
#include <vector>

namespace utbot {
    class RunCommand : public BaseCommand {
    public:
        RunCommand(std::vector<std::string> commandLine, fs::path directory);
        fs::path getOutput() const override;

        std::string toStringWithChangingDirectory() const override;

        static RunCommand forceRemoveFile(fs::path const &path, fs::path const &workingDir);

        static RunCommand
        copyFile(fs::path const &from, const fs::path &to, fs::path const &workingDir);

        bool isArchiveCommand() const override;
    };
}


#endif // UNITTESTBOT_RUNCOMMAND_H
