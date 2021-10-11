/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_LINKCOMMAND_H
#define UNITTESTBOT_LINKCOMMAND_H

#include "BaseCommand.h"

#include "utils/path/FileSystemPath.h"
#include <list>
#include <string>

namespace utbot {
    class LinkCommand : public BaseCommand {
    private:
        iterator linker;
        iterator output;

    public:
        LinkCommand() = default;

        LinkCommand(LinkCommand const &other);

        LinkCommand &operator=(LinkCommand const &other);

        LinkCommand(LinkCommand &&other) noexcept;

        LinkCommand &operator=(LinkCommand &&other) noexcept;

        LinkCommand(std::list<std::string> commandLine, fs::path directory);

        LinkCommand(std::vector<std::string> commandLine, fs::path directory);

        LinkCommand(std::initializer_list<std::string> commandLine, fs::path directory);

        friend void swap(LinkCommand &a, LinkCommand &b) noexcept;

        [[nodiscard]] fs::path getLinker() const;

        void setLinker(fs::path linker);

        [[nodiscard]] fs::path getOutput() const override;

        void setOutput(fs::path output);

        [[nodiscard]] bool isArchiveCommand() const override;

        [[nodiscard]] bool isSharedLibraryCommand() const;
    };
}


#endif //UNITTESTBOT_LINKCOMMAND_H
