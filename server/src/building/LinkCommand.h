#ifndef UNITTESTBOT_LINKCOMMAND_H
#define UNITTESTBOT_LINKCOMMAND_H

#include "BaseCommand.h"

#include "utils/path/FileSystemPath.h"
#include <list>
#include <string>

namespace utbot {
    class LinkCommand : public BaseCommand {
    private:
        void initOutput();

    public:
        LinkCommand() = default;

        LinkCommand(LinkCommand const &other);

        LinkCommand &operator=(LinkCommand const &other);

        LinkCommand(LinkCommand &&other) noexcept;

        LinkCommand &operator=(LinkCommand &&other) noexcept;

        LinkCommand(std::list<std::string> commandLine, fs::path directory, bool shouldChangeDirectory = false);

        LinkCommand(std::vector<std::string> commandLine, fs::path directory, bool shouldChangeDirectory = false);

        LinkCommand(std::initializer_list<std::string> commandLine, fs::path directory,
                    bool shouldChangeDirectory = false);

        friend void swap(LinkCommand &a, LinkCommand &b) noexcept;

        [[nodiscard]] bool isArchiveCommand() const override;

        [[nodiscard]] bool isSharedLibraryCommand() const;
    };
}


#endif //UNITTESTBOT_LINKCOMMAND_H
