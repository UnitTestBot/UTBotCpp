/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "CompileCommand.h"

#include "BaseCommand.h"
#include "Paths.h"
#include "printers/CCJsonPrinter.h"
#include "utils/StringUtils.h"

#include "loguru.h"

#include <algorithm>
#include <iterator>
#include <set>
#include <utility>

namespace utbot {
    // See https://gcc.gnu.org/onlinedocs/gcc/Option-Summary.html
    static const std::unordered_set<std::string> gccSpecificOptions = {
        "-std", "-fpermitted-flt-eval-methods", "-fopenacc-dim", "-fopenacc-kernels", "-fsso-struct"
    };

    static const std::unordered_set<std::string> gccSpecificFlags = {
        "-ansi", "-fgnu89-inline",
        "-fpermitted-flt-eval-methods",
        "-fallow-parameterless-variadic-functions",
        "-fno-asm",
        "-fno-builtin",
        "-fno-builtin-function", "-fgimple",
        "-fhosted",
        "-ffreestanding",
        "-fopenacc",
        "-fopenmp", "-fopenmp-simd",
        "-fms-extensions", "-fplan9-extensions",
        "-fallow-single-precision", "-fcond-mismatch", "-flax-vector-conversions",
        "-fsigned-bitfields", "-fsigned-char",
        "-funsigned-bitfields", "-funsigned-char"
    };

    CompileCommand::CompileCommand(CompileCommand const &other) : BaseCommand(other) {
        compiler = commandLine.begin();
        sourcePath =
            std::next(commandLine.begin(),
                      std::distance<const_iterator>(other.commandLine.begin(), other.sourcePath));
        output = std::next(commandLine.begin(),
                           std::distance<const_iterator>(other.commandLine.begin(), other.output));
    }

    CompileCommand::CompileCommand(CompileCommand &&other) noexcept : BaseCommand(std::move(other)),
                                                                      sourcePath(other.sourcePath),
                                                                      compiler(other.compiler),
                                                                      output(other.output) {
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

    CompileCommand::CompileCommand(std::vector<std::string> arguments,
                                   fs::path directory,
                                   fs::path sourcePath)
        : BaseCommand(std::move(arguments), std::move(directory)) {
        compiler = commandLine.begin();
        {
            auto it = std::find_if(commandLine.begin(), commandLine.end(), [&sourcePath](std::string const &arg) {
                return fs::path(arg).filename() == sourcePath.filename();
            });
            this->sourcePath = it;
            *this->sourcePath = sourcePath;
        }
        {
            auto it = findOutput();
            if (it != commandLine.end()) {
                this->output = it;
                *this->output = Paths::getCCJsonFileFullPath(*it, this->directory);
            } else {
                auto path = Paths::getCCJsonFileFullPath(Paths::replaceExtension(*this->sourcePath, ".o"), this->directory);
                this->output = std::next(addFlagsToBegin({ "-o", path }));
            }
        }
    }

    void swap(CompileCommand &a, CompileCommand &b) noexcept {
        std::swap(a.directory, b.directory);
        std::swap(a.commandLine, b.commandLine);
        std::swap(a.environmentVariables, b.environmentVariables);
        std::swap(a.optimizationLevel, b.optimizationLevel);

        std::swap(a.sourcePath, b.sourcePath);
        std::swap(a.compiler, b.compiler);
        std::swap(a.output, b.output);
    }

    fs::path CompileCommand::getSourcePath() const {
        return *sourcePath;
    }

    void CompileCommand::setSourcePath(fs::path sourcePath) {
        *(this->sourcePath) = std::move(sourcePath);
    }

    fs::path CompileCommand::getCompiler() const {
        return *compiler;
    }

    void CompileCommand::setCompiler(fs::path compiler) {
        *(this->compiler) = std::move(compiler);
    }

    fs::path CompileCommand::getOutput() const {
        return *output;
    }

    bool CompileCommand::isArchiveCommand() const {
        return false;
    }

    void CompileCommand::setOutput(fs::path output) {
        *(this->output) = std::move(output);
    }

    void CompileCommand::removeGccFlags() {
        CollectionUtils::erase(commandLine, "--coverage");
        CollectionUtils::erase(commandLine, "-fprofile-dir=.");
    }

    void CompileCommand::filterCFlags() {
        size_t erased = CollectionUtils::erase_if(commandLine, [](std::string const &arg) {
            size_t pos = arg.find('=');
            if (pos != std::string::npos) {
                return CollectionUtils::contains(gccSpecificOptions, arg.substr(0, pos));
            }
            return CollectionUtils::contains(gccSpecificFlags, arg);
        });
        LOG_S(DEBUG) << erased << " C specific flags erased from compile arguments";
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
}