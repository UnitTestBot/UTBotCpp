#ifndef UNITTESTBOT_BUILDDATABASE_H
#define UNITTESTBOT_BUILDDATABASE_H

#include "ProjectContext.h"
#include "building/CompileCommand.h"
#include "building/LinkCommand.h"
#include "utils/CollectionUtils.h"

#include "json.hpp"
#include <tsl/ordered_set.h>

#include "utils/path/FileSystemPath.h"
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

class BuildDatabase {
public:
    static const std::string BITS_32_FLAG;

    struct KleeFilesInfo {
        explicit KleeFilesInfo(fs::path kleeFile);

        void setCorrectMethods(std::unordered_set<std::string> correctMethods);

        void setAllAreCorrect(bool allAreCorrect);

        bool isCorrectMethod(const std::string &method);

        fs::path getKleeFile();

        fs::path getKleeBitcodeFile();

        fs::path getKleeFile(const std::string &methodName);

        fs::path getKleeBitcodeFile(const std::string &methodName);

    private:
        fs::path kleeFile;
        std::unordered_set<std::string> correctMethods;
        bool allAreCorrect = false;
    };

    struct BaseFileInfo {
        BaseFileInfo() = default;

        virtual ~BaseFileInfo() = default;

        // Object files and libraries that current command depends on
        CollectionUtils::OrderedFileSet files;

        // Libraries that current command depends on, but those are already installed and not built
        // within project
        CollectionUtils::OrderedFileSet installedFiles;

        void addFile(fs::path file);
    };

    /*
     * struct ObjectFileInfo represents a compile command from compile_commands.json
     * that produces an object file
     */
    struct ObjectFileInfo : BaseFileInfo {
        // Compilation command
        utbot::CompileCommand command;

        // Example of executable or a library which contains current objectFile
        fs::path linkUnit;

        // Information about klee files
        std::shared_ptr<KleeFilesInfo> kleeFilesInfo;

        // Directory from where to execute the command
        [[nodiscard]] fs::path const &getDirectory() const;

        // User source file
        [[nodiscard]] fs::path getSourcePath() const;

        // User object file
        [[nodiscard]] fs::path getOutputFile() const;

        [[nodiscard]] bool is32bits() const;

        void setOutputFile(const fs::path &file);

        static bool conflictPriorityMore(const std::shared_ptr<ObjectFileInfo> &left,
                                         const std::shared_ptr<ObjectFileInfo> &right);
    };

    /*
    * struct TargetInfo represents
    * a link command from link_commands.json
    * OR
    * a compile command from compile_commands.json that produces an executable
    */
    struct TargetInfo : BaseFileInfo {
        // Linkage command
        std::vector<utbot::LinkCommand> commands;

        // Units which contains current library
        std::vector<fs::path> parentLinkUnits;

        // Executable or a library, the result of a command
        [[nodiscard]] fs::path getOutput() const;
    };

public:
    const fs::path &getCompileCommandsJson();

    const fs::path &getLinkCommandsJson();

    /**
     * @brief Returns all object files that are contained in a library or executable
     *
     * Recursively iterates over all libraries inside current library or executable. Returns all
     * found object files
     * @param archive Executable or a library for which to find object files
     * @return Set of paths to object files.
     * @throws CompilationDatabaseException if files are wrong
     */
    [[nodiscard]] CollectionUtils::FileSet getArchiveObjectFiles(const fs::path &archive) const;

    /**
     * @brief Returns all target that are contained in a library or executable
     *
     * Recursively iterates over all libraries inside current library or executable. Returns all
     * found target
     * @param archive Executable or a library for which to find target files
     * @return Set of paths to targets path.
     * @throws CompilationDatabaseException if files are wrong
     */
    [[nodiscard]] CollectionUtils::FileSet getArchiveTargetFiles(const fs::path &archive) const;

    /**
     * @brief Returns compile command information for object file
     *
     * @param filepath Path to object file
     * @return ObjectFileInfo for object file
     * @throws CompilationDatabaseException if files are wrong
     */
    [[nodiscard]] std::shared_ptr<ObjectFileInfo> getClientCompilationObjectInfo(const fs::path &filepath) const;

    /**
     * @brief Returns compile command information for current source file
     *
     * @param filepath Path to source file or object file
     * @return ObjectFileInfo for current source file
     * @throws CompilationDatabaseException if files are wrong
     */
    [[nodiscard]] std::shared_ptr<ObjectFileInfo> getClientCompilationSourceInfo(const fs::path &filepath) const;

    /**
     * @brief Returns compile command information for current source file or object file
     *
     * @param filepath Path to source file or object file
     * @return ObjectFileInfo for current source file or object file
     * @throws CompilationDatabaseException if files are wrong
     */
    [[nodiscard]] std::shared_ptr<const ObjectFileInfo> getClientCompilationUnitInfo(const fs::path &filepath) const;

    /**
     * @brief Returns true if BuildDatabase contains information for current source file or object file
     *
     * @param filepath Path to source file or object file
     * @return true if contains current source file or object file
     */
    [[nodiscard]] bool hasUnitInfo(const fs::path &filepath) const;

    /**
     * @brief Returns link command information for current executable or library
     *
     * @param filepath Path to executable or library
     * @return TargetInfo for current executable or library
     * @throws CompilationDatabaseException if files are wrong
     */
    [[nodiscard]] std::shared_ptr<const TargetInfo> getClientLinkUnitInfo(const fs::path &filepath) const;

    [[nodiscard]] fs::path getObjectFile(const fs::path &sourceFile) const;

    /**
     * @brief Returns example of executable or library for current source file or library
     *
     * @param filepath Path to source file or library
     * @return path Path to executable or library
     * @throws CompilationDatabaseException if file is wrong
     */
    [[nodiscard]] fs::path getRootForSource(const fs::path &path) const;

    [[nodiscard]] fs::path getRootForFirstSource() const;

    [[nodiscard]] fs::path getBitcodeForSource(const fs::path &sourceFile) const;

    [[nodiscard]] fs::path getBitcodeFile(const fs::path &filepath) const;

    /**
     * @brief Checks if object file is in priority for it's corresponding source file
     *
     * Source files may be compiled several times to different object files.
     * Tests are generated only for one object file for now. This file is chosen according to heuristics.
     * For example, if a filename contains 64 in it's name, it is considered as a file for x64 architecture and gets more priority.
     * This method checks if current object file is in max priority for it's corresponding source file.
     * @param objectFilePath Path to object file
     * @return path Path to bitcode file of executable or library
     * @throws CompilationDatabaseException if file is wrong
     */
    bool isFirstObjectFileForSource(const fs::path &objectFilePath) const;

    /**
     * @brief Gets all compilation commands
     *
     * @return Vector of pointers to ObjectFileInfo
     */
    std::vector<std::shared_ptr<ObjectFileInfo>> getAllCompileCommands() const;

    virtual std::vector<std::shared_ptr<TargetInfo>> getRootTargets() const;

    virtual std::vector<std::shared_ptr<TargetInfo>> getAllTargets() const;

    virtual std::vector<fs::path> getAllTargetPaths() const;

    virtual std::vector<fs::path> getTargetPathsForSourceFile(const fs::path &sourceFilePath) const;

    virtual std::vector<fs::path> getTargetPathsForObjectFile(const fs::path &objectFile) const;

    std::shared_ptr<TargetInfo> getPriorityTarget() const;

    CollectionUtils::FileSet getSourceFilesForTarget(const fs::path &_target);

    std::shared_ptr<TargetInfo> getTargetInfo(const fs::path &_target);

    std::shared_ptr<CompilationDatabase> compilationDatabase;

protected:
    const fs::path serverBuildDir;
    const fs::path buildCommandsJsonPath;
    const fs::path linkCommandsJsonPath;
    const fs::path compileCommandsJsonPath;
    const utbot::ProjectContext projectContext;
    CollectionUtils::MapFileTo<std::vector<std::shared_ptr<ObjectFileInfo>>> sourceFileInfos;
    CollectionUtils::MapFileTo<std::shared_ptr<ObjectFileInfo>> objectFileInfos;
    CollectionUtils::FileSet ignoredOutput;
    CollectionUtils::MapFileTo<std::shared_ptr<TargetInfo>> targetInfos;
    CollectionUtils::MapFileTo<std::vector<fs::path>> objectFileTargets;

    BuildDatabase(
            fs::path serverBuildDir,
            fs::path buildCommandsJsonPath,
            fs::path linkCommandsJsonPath,
            fs::path compileCommandsJsonPath,
            utbot::ProjectContext projectContext
    );

    BuildDatabase(BuildDatabase *baseBuildDatabase);

    static fs::path getCorrespondingBitcodeFile(const fs::path &filepath);

    void createClangCompileCommandsJson();

    void mergeLibraryOptions(std::vector<std::string> &jsonArguments) const;

    fs::path newDirForFile(fs::path const &file) const;

    fs::path createExplicitObjectFileCompilationCommand(const std::shared_ptr<ObjectFileInfo> &objectInfo);

    std::vector<std::shared_ptr<TargetInfo>> getTargetsForSourceFile(fs::path const &sourceFilePath) const;

    using sharedLibrariesMap = std::unordered_map<std::string, CollectionUtils::MapFileTo<fs::path>>;

    void addLibrariesForCommand(utbot::BaseCommand &command,
                                BaseFileInfo &info,
                                sharedLibrariesMap &sharedLibraryFiles,
                                bool objectFiles = false);
};

#endif //UNITTESTBOT_BUILDDATABASE_H
