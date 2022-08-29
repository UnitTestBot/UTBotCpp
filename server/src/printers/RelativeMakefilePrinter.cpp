#include "environment/EnvironmentPaths.h"
#include "RelativeMakefilePrinter.h"
#include "utils/StringFormat.h"

namespace printer {

RelativeMakefilePrinter::RelativeMakefilePrinter(PathToShellVariable pathToShellVariable)
    : DefaultMakefilePrinter(),
      pathToShellVariable{pathToShellVariable} { }

RelativeMakefilePrinter::RelativeMakefilePrinter(
        const fs::path &buildDirectory,
        const fs::path &buildDirectoryRelative,
        const fs::path &projectPath)
        : DefaultMakefilePrinter(),
          pathToShellVariable{{{buildDirectory, "$(BUILD_DIR)"},
                               {projectPath, "$(PROJECT_DIR)"},
                               {buildDirectoryRelative, "$(BUILD_RELATIVE)"}},
                              [](const std::string& lhs, const std::string& rhs) -> bool {
              if (rhs == Paths::getUTBotRootDir().string()) {
                  return true;
              }
              return std::greater<>()(lhs, rhs);
          }},
          buildDirectoryRelative{buildDirectoryRelative},
          projectPath{projectPath} {
    initializePathsToShellVariables();
}

fs::path RelativeMakefilePrinter::getRelativePath(fs::path source) const {
    return getRelativePath(source, true);
}

fs::path RelativeMakefilePrinter::getRelativePath(fs::path source, bool isCanonical) const {
    if (isCanonical) {
        source = fs::weakly_canonical(source);
    }

    for (const auto &[path, shellVariable] : pathToShellVariable) {
        std::optional<fs::path> relative = Paths::getRelativePathWithShellVariable(
                shellVariable,
                path,
                source);
        if (relative.has_value()) {
            return relative.value();
        }
    }
    // don't change path if can't convert to relative
    return source;
}

void RelativeMakefilePrinter::declareShellVariable(const std::string& variableName, fs::path path,
                                                std::function<void(const std::string&, const std::string&)> shellVariableDeclarationFunction,
                                                bool shouldWriteToMap, bool isCanonical) {
    const fs::path relativePath = getRelativePath(path, isCanonical);
    if (isCanonical) {
        path = fs::weakly_canonical(path);
    }
    shellVariableDeclarationFunction(variableName, relativePath.string());
    if (shouldWriteToMap) {
        pathToShellVariable[path] = StringUtils::stringFormat("$(%s)", variableName);
    }
}

std::string RelativeMakefilePrinter::getProjectStructureRelativeTo(fs::path path) const {
    DefaultMakefilePrinter printer;
    printer.declareVariable("export PROJECT_DIR_RELATIVE_TO_MAKEFILE", fs::relative(projectPath, path));
    // magic spell from https://stackoverflow.com/questions/18136918/how-to-get-current-relative-directory-of-your-makefile
    printer.declareVariable("export MAKEFILE_DIR", "$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))");
    printer.declareVariable("export PROJECT_DIR", "$(MAKEFILE_DIR)/$(PROJECT_DIR_RELATIVE_TO_MAKEFILE)");
    printer.declareVariable("export BUILD_RELATIVE", buildDirectoryRelative);
    printer.declareVariable("export BUILD_DIR", "$(PROJECT_DIR)/$(BUILD_RELATIVE)");
    return printer.ss.str();
}

void RelativeMakefilePrinter::initializePathsToShellVariables() {
    const auto declareVariableFunc = [&](const std::string &arg1, const std::string &arg2) {
        declareVariable(arg1, arg2);
    };
    const auto declareVariableIfNotDefinedFunc = [&](const std::string &arg1, const std::string &arg2) {
        declareVariableIfNotDefined(arg1, arg2);
    };
    const auto declareVariableWithPriorityFunc = [&](const std::string &arg1, const std::string &arg2) {
        declareVariableWithPriority(arg1, arg2);
    };

    declareShellVariable("UTBOT_ALL", Paths::getUTBotRootDir(), declareVariableIfNotDefinedFunc);

    declareShellVariable("UTBOT_INSTALL", Paths::getUTBotInstallDir(), declareVariableFunc);

    declareShellVariable("ACCESS_PRIVATE_LIB", Paths::getAccessPrivateLibPath(), declareVariableFunc);

    declareShellVariable("UTBOT_DEBS_INSTALL_DIR", Paths::getUTBotDebsInstallDir(), declareVariableFunc);

    declareShellVariable("ASAN_LIB", Paths::getAsanLibraryPath(), declareVariableFunc);

    declareShellVariable("CLANG", Paths::getUTBotClang(), declareVariableWithPriorityFunc);

    declareShellVariable("CLANGXX", Paths::getUTBotClangPP(), declareVariableWithPriorityFunc, false, false);

    pathToShellVariable[Paths::getUTBotClangPP()] = "$(CLANGXX)";

    declareShellVariable("GCC", Paths::getGcc(), declareVariableFunc);
    declareShellVariable("GXX", Paths::getGpp(), declareVariableFunc);

    declareShellVariable("AR", Paths::getAr(), declareVariableIfNotDefinedFunc);

    declareShellVariable("LD_GOLD", Paths::getLdGold(), declareVariableIfNotDefinedFunc);

    declareShellVariable("LD", Paths::getLd(), declareVariableIfNotDefinedFunc);

    declareShellVariable("GTEST", Paths::getGtestLibPath(), declareVariableWithPriorityFunc);
}

void RelativeMakefilePrinter::declareVariable(std::string const &name, std::string const &value) {
    ss << StringUtils::stringFormat("export %s = %s\n", name, value);
}

fs::path RelativeMakefilePrinter::getRelativePathForLinker(fs::path path) const {
    const auto compiler = CompilationUtils::getCompilerName(path);
    if (compiler != CompilationUtils::CompilerName::CLANGXX) {
        return getRelativePath(path);
    }
    // don't transform to canonical if CLANGXX
    return getRelativePath(path, false);
}

}
