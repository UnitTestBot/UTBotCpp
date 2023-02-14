#include "CompileCommand.h"

#include "Paths.h"
#include "printers/CCJsonPrinter.h"
#include "utils/StringUtils.h"

#include "loguru.h"

#include <algorithm>
#include <set>
#include <utility>

namespace utbot {
    CompileCommand::CompileCommand(CompileCommand const &other) : BaseCommand(other) {
        sourcePath =
            std::next(commandLine.begin(),
                      std::distance<const_iterator>(other.commandLine.begin(), other.sourcePath));
        output = std::next(commandLine.begin(),
                           std::distance<const_iterator>(other.commandLine.begin(), other.output));
    }

    CompileCommand::CompileCommand(CompileCommand &&other) noexcept : BaseCommand(std::move(other)),
                                                                      sourcePath(other.sourcePath) {
    }

    CompileCommand &CompileCommand::operator=(const CompileCommand &other) {
        if (this == &other) {
            return *this;
        }
        CompileCommand tmp(other);
        swap(*this, tmp);
        return *this;
    }

    CompileCommand &CompileCommand::operator=(CompileCommand &&other) noexcept {
        if (this == &other) {
            return *this;
        }
        CompileCommand tmp(std::move(other));
        swap(*this, tmp);
        return *this;
    }

    CompileCommand::CompileCommand(const CompileCommand &other, bool shouldChangeDirectory_) :
                                   CompileCommand(other) {
        shouldChangeDirectory = shouldChangeDirectory_;
    }

    CompileCommand::CompileCommand(std::vector<std::string> arguments,
                                   fs::path directory,
                                   fs::path sourcePath)
        : BaseCommand(std::move(arguments), std::move(directory)) {
        {
            auto it = std::find_if(commandLine.begin(), commandLine.end(), [&sourcePath](std::string const &arg) {
                return fs::path(arg).filename() == sourcePath.filename();
            });
            this->sourcePath = it;
            *this->sourcePath = sourcePath;
        }
        initOutput();
    }

    void swap(CompileCommand &a, CompileCommand &b) noexcept {
        std::swap(a.directory, b.directory);
        std::swap(a.commandLine, b.commandLine);
        std::swap(a.environmentVariables, b.environmentVariables);
        std::swap(a.optimizationLevel, b.optimizationLevel);

        std::swap(a.sourcePath, b.sourcePath);
        std::swap(a.output, b.output);
    }

    fs::path CompileCommand::getSourcePath() const {
        return *sourcePath;
    }

    void CompileCommand::setSourcePath(fs::path sourcePath) {
        *(this->sourcePath) = std::move(sourcePath);
    }

    bool CompileCommand::isArchiveCommand() const {
        return false;
    }

    void CompileCommand::removeCompilerFlagsAndOptions(
        const std::unordered_set<std::string> &switchesToRemove) {
        size_t erased =
            CollectionUtils::erase_if(commandLine, [&switchesToRemove](std::string const &arg) {
                size_t pos = arg.find('=');
                const std::string &toFind = pos == std::string::npos ? arg : arg.substr(0, pos);
                return CollectionUtils::contains(switchesToRemove, toFind);
            });
        LOG_S(DEBUG) << erased << " Compiler specific switches erased from compile arguments";
    }

    void CompileCommand::removeIncludeFlags() {
        CollectionUtils::erase_if(commandLine, [](const std::string &arg) {
            return StringUtils::startsWith(arg, "-I");
        });
    }

    void CompileCommand::removeWerror() {
        CollectionUtils::erase_if(commandLine, [](const std::string &arg) {
            return StringUtils::startsWith(arg, "-Werror");
        });
    }

    void CompileCommand::initOutput() {
        auto it = findOutput();
        if (it != commandLine.end()) {
            this->output = it;
            *this->output = Paths::getCCJsonFileFullPath(*it, this->directory);
        } else {
            auto path = Paths::getCCJsonFileFullPath(Paths::replaceExtension(*this->sourcePath, ".o"), this->directory);
            this->output = std::next(addFlagsToBegin({"-o", path}));
        }
    }
}
