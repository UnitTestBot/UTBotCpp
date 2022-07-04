#include "BaseCommand.h"

#include "CompileCommand.h"
#include "LinkCommand.h"
#include "Paths.h"
#include "printers/CCJsonPrinter.h"
#include "tasks/ShellExecTask.h"
#include "utils/CollectionUtils.h"
#include "utils/StringUtils.h"

#include "loguru.h"

#include <algorithm>
#include "utils/path/FileSystemPath.h"
#include <iterator>
#include <set>
#include <utility>

namespace utbot {
    BaseCommand::BaseCommand(std::list<std::string> commandLine, fs::path directory)
        : commandLine(std::move(commandLine)), directory(std::move(directory)) {
        initOptimizationLevel();
    }
    BaseCommand::BaseCommand(std::vector<std::string> commandLine, fs::path directory)
        : commandLine(commandLine.begin(), commandLine.end()), directory(std::move(directory)) {
        initOptimizationLevel();
    }

    BaseCommand::BaseCommand(BaseCommand const &other)
        : directory(other.directory), commandLine(other.commandLine),
          environmentVariables(other.environmentVariables) {
        if (other.optimizationLevel.has_value()) {
            optimizationLevel =
                std::next(commandLine.begin(),
                          std::distance<const_iterator>(other.commandLine.begin(),
                                                        other.optimizationLevel.value()));
        }
    }
    BaseCommand::BaseCommand(BaseCommand &&other) noexcept
        : directory(std::move(other.directory)), commandLine(std::move(other.commandLine)),
          environmentVariables(std::move(other.environmentVariables)),
          optimizationLevel(other.optimizationLevel) {
    }

    void BaseCommand::initOptimizationLevel() {
        auto it = findOptimizationLevelFlag();
        if (it != commandLine.end()) {
            optimizationLevel = it;
        }
    }
    BaseCommand::iterator BaseCommand::findOutput() {
        auto it = std::find(commandLine.begin(), commandLine.end(), "-o");
        if (it != commandLine.end()) {
            return std::next(it, 1);
        }
        return it;
    }

    BaseCommand::iterator BaseCommand::findOptimizationLevelFlag() {
        return std::find_if(commandLine.begin(), commandLine.end(), [](std::string const &flag) {
            return StringUtils::startsWith(flag, "-O");
        });
    }

    BaseCommand::iterator BaseCommand::addFlagToBegin(std::string flag) {
        return commandLine.insert(std::next(commandLine.begin()), std::move(flag));
    }

    BaseCommand::iterator BaseCommand::addFlagToEnd(std::string flag) {
        return commandLine.insert(std::end(commandLine), std::move(flag));
    }

    void BaseCommand::addEnvironmentVariable(std::string name, std::string value) {
        environmentVariables.insert_or_assign(std::move(name), std::move(value));
    }

    std::string BaseCommand::toString() const {
        auto command = StringUtils::joinWith(commandLine, " ");
        if (environmentVariables.empty()) {
            return command;
        }
        auto environment = StringUtils::joinWith(
            CollectionUtils::transformTo<std::vector<std::string>>(
                environmentVariables, [](auto const &it) { return it.first + "=" + it.second; }),
            " ");
        return environment + " " + command;
    }

    std::list<std::string> &BaseCommand::getCommandLine() {
        return commandLine;
    }

    const std::list<std::string> &BaseCommand::getCommandLine() const {
        return commandLine;
    }

    const fs::path &utbot::BaseCommand::getDirectory() const {
        return directory;
    }

    bool BaseCommand::replace(const fs::path &from, const fs::path &to) {
        return CollectionUtils::replace(commandLine, from, to);
    }
    bool BaseCommand::erase(std::string const &arg) {
        return CollectionUtils::erase(commandLine, arg);
    }
    size_t BaseCommand::erase_if(std::function<bool(std::string)> f) {
        return CollectionUtils::erase_if(commandLine, f);
    }
    std::string BaseCommand::toStringWithChangingDirectory() const {
        std::string baseCommand = toString();
        return StringUtils::stringFormat(CompilationUtils::FULL_COMMAND_PATTERN, directory,
                                         getOutput().parent_path(), baseCommand);
    }

    void BaseCommand::setOptimizationLevel(const std::string &flag) {
        if (optimizationLevel.has_value()) {
            *(optimizationLevel.value()) = flag;
        } else {
            optimizationLevel = addFlagToBegin(flag);
        }
    }
}
