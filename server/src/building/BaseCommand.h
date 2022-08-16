#ifndef UNITTESTBOT_BASECOMMAND_H
#define UNITTESTBOT_BASECOMMAND_H

#include "tasks/ShellExecTask.h"

#include <tsl/ordered_map.h>

#include "utils/path/FileSystemPath.h"
#include <forward_list>
#include <list>
#include <optional>
#include <string>
#include <vector>

namespace utbot {
    class BaseCommand {
    protected:
        bool shouldChangeDirectory = false;
        fs::path directory;
        std::list<std::string> commandLine{};
        tsl::ordered_map<std::string, std::string> environmentVariables{};

        using iterator = decltype(commandLine)::iterator;
        using const_iterator = decltype(commandLine)::const_iterator;

        iterator buildTool;
        iterator output;

        std::optional<iterator> optimizationLevel;

        void initOptimizationLevel();

        void initBuildTool();

        void initOutput();

        [[nodiscard]] iterator findOutput();

        iterator findOptimizationLevelFlag();


        BaseCommand() = default;

        BaseCommand(std::list<std::string> commandLine, fs::path directory, bool shouldChangeDirectory = false);

        BaseCommand(std::vector<std::string> commandLine, fs::path directory, bool shouldChangeDirectory = false);

        BaseCommand(BaseCommand const &other);

        BaseCommand(BaseCommand &&other) noexcept;

    public:

        [[nodiscard]] std::list<std::string> &getCommandLine();

        [[nodiscard]] const std::list<std::string> &getCommandLine() const;

        [[nodiscard]] const fs::path &getDirectory() const;

        [[nodiscard]] fs::path getOutput() const;

        void setOutput(fs::path output);

        [[nodiscard]] virtual bool isArchiveCommand() const = 0;

        iterator addFlagToBegin(std::string flag);

        template<typename ContainerT = std::initializer_list<std::string>, typename IteratorT = typename ContainerT::iterator>
        iterator addFlagsBeforeIterator(ContainerT&& flags, IteratorT&& it) {
            return commandLine.insert(it, std::begin(flags), std::end(flags));
        }

        template<typename ContainerT = std::initializer_list<std::string>>
        iterator addFlagsToBegin(ContainerT&& flags) {
            return commandLine.insert(std::next(commandLine.begin()), std::begin(flags),
                                      std::end(flags));
        }

        iterator addFlagToEnd(std::string flags);

        template<typename ContainerT = std::initializer_list<std::string>>
        iterator addFlagsToEnd(ContainerT&& flags) {
            return commandLine.insert(std::end(commandLine), std::begin(flags), std::end(flags));
        }

        void addEnvironmentVariable(std::string name, std::string value);

        [[nodiscard]] std::string toString() const;

        [[nodiscard]] ShellExecTask::ExecutionParameters toExecutionParameters() const;

        [[nodiscard]] std::string toStringWithChangingDirectory() const;

        [[nodiscard]] virtual std::string toStringWithChangingDirectoryToNew(const fs::path &targetDirectory) const;

        bool replace(fs::path const &from, fs::path const &to);

        bool erase(std::string const& arg);

        size_t erase_if(std::function<bool(std::string)> f);

        void setOptimizationLevel(const std::string &flag);

        [[nodiscard]] fs::path getBuildTool() const;

        void setBuildTool(fs::path buildTool);
    };
}


#endif // UNITTESTBOT_BASECOMMAND_H
