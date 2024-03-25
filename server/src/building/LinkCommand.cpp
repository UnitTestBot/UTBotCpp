#include "LinkCommand.h"

#include "BaseCommand.h"
#include "Paths.h"
#include "utils/CollectionUtils.h"
#include "utils/StringUtils.h"

#include "utils/path/FileSystemPath.h"
#include <iterator>
#include <utility>

namespace utbot {
    LinkCommand::LinkCommand(LinkCommand const &other) : BaseCommand(other) {
        output = std::next(commandLine.begin(), std::distance<const_iterator>(other.commandLine.begin(), other.output));
    }

    LinkCommand::LinkCommand(LinkCommand &&other) noexcept: BaseCommand(std::move(other)) {
    }

    LinkCommand &LinkCommand::operator=(const LinkCommand &other) {
        if (this == &other) {
            return *this;
        }
        LinkCommand tmp(other);
        swap(*this, tmp);
        return *this;
    }

    LinkCommand &LinkCommand::operator=(LinkCommand &&other) noexcept {
        if (this == &other) {
            return *this;
        }
        LinkCommand tmp(std::move(other));
        swap(*this, tmp);
        return *this;
    }

    LinkCommand::LinkCommand(std::list<std::string> arguments, fs::path directory, bool shouldChangeDirectory)
            : BaseCommand(std::move(arguments), std::move(directory), shouldChangeDirectory) {
        initOutput();
    }

    LinkCommand::LinkCommand(std::vector<std::string> commandLine, fs::path directory, bool shouldChangeDirectory)
            : LinkCommand(std::list<std::string>{commandLine.begin(), commandLine.end()},
                          std::move(directory), shouldChangeDirectory) {
    }

    LinkCommand::LinkCommand(std::initializer_list<std::string> commandLine, fs::path directory,
                             bool shouldChangeDirectory)
            : LinkCommand(std::list<std::string>{commandLine.begin(), commandLine.end()},
                          std::move(directory), shouldChangeDirectory) {
    }


    void swap(LinkCommand &a, LinkCommand &b) noexcept {
        std::swap(a.directory, b.directory);
        std::swap(a.commandLine, b.commandLine);
        std::swap(a.environmentVariables, b.environmentVariables);
        std::swap(a.optimizationLevel, b.optimizationLevel);

        std::swap(a.output, b.output);
    }

    bool LinkCommand::isArchiveCommand() const {
        return StringUtils::contains(getBuildTool().filename().c_str(), "ld") ||
               StringUtils::contains(getBuildTool().filename().c_str(), "ar");
    }

    bool LinkCommand::isSharedLibraryCommand() const {
        return StringUtils::contains(getBuildTool().filename().c_str(), "ld") ||
               CollectionUtils::contains(commandLine, "-shared");
    }

    void LinkCommand::initOutput() {
        auto it = findOutput();
        if (it != commandLine.end()) {
            this->output = it;
            *this->output = Paths::getFileFullPath(*it, this->directory);
        } else if (isArchiveCommand()) {
            it = std::find_if(commandLine.begin(), commandLine.end(), [](const std::string &argument) {
                return Paths::isStaticLibraryFile(argument);
            });
            this->output = std::next(addFlagsBeforeIterator({"-o"}, it));
        } else {
            auto path = this->directory / "a.out";
            this->output = std::next(addFlagsToBegin({"-o", path}));
        }
    }
}
