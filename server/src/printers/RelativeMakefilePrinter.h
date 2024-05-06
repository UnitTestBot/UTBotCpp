#ifndef UNITTESTBOT_RELATIVEMAKEFILEPRINTER_H
#define UNITTESTBOT_RELATIVEMAKEFILEPRINTER_H

#include "printers/DefaultMakefilePrinter.h"

namespace printer {
class RelativeMakefilePrinter : public DefaultMakefilePrinter {
public:
    using PathToShellVariable =
            std::map<std::string, fs::path, std::function<bool(const std::string&, const std::string&)>>;

    RelativeMakefilePrinter(PathToShellVariable pathToShellVariable);
    RelativeMakefilePrinter(const fs::path &buildDirectory,
                            const fs::path &buildDirectoryRelative,
                            const fs::path &projectPath,
                            const fs::path &testsPath);

    ~RelativeMakefilePrinter() override = default;

    void declareVariable(std::string const &name, std::string const &value) override;

private:
    fs::path getRelativePath(fs::path source, bool isCanonical) const;
    void initializePathsToShellVariables();

protected:
    fs::path buildDirectoryRelative, testsPath, projectPath;

    // map variable with absolute path to $(someVar)
    std::map<std::string, fs::path, std::function<bool(const std::string&, const std::string&)>> pathToShellVariable;
    fs::path getRelativePath(const fs::path &source) const;
    void declareShellVariable(const std::string& variableName, fs::path path,
                              std::function<void(const std::string&, const std::string&)> shellVariableDeclarationFunction,
                              bool shouldWriteToMap = true, bool isCanonical = true);
    fs::path getRelativePathForLinker(fs::path path) const;
    std::string getProjectStructureRelativeTo(const fs::path &makefilePath) const;
};
}




#endif //UNITTESTBOT_RELATIVEMAKEFILEPRINTER_H
