/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_BUILDDATABASE_H
#define UNITTESTBOT_BUILDDATABASE_H

#include "ProjectContext.h"
#include "building/CompileCommand.h"
#include "building/LinkCommand.h"
#include "utils/CollectionUtils.h"

#include <nlohmann/json.hpp>
#include <tsl/ordered_set.h>

#include "utils/path/FileSystemPath.h"
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

class BuildDatabase {
public:
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

    /*
     * struct ObjectFileInfo represents a compile command from compile_commands.json
     * that produces an object file
     */
    struct ObjectFileInfo {
        utbot::CompileCommand command; // Compilation command

        tsl::ordered_set<fs::path, HashUtils::PathHash> files; // Object files and libraries that current command depends on
        fs::path linkUnit;                            // Example of executable or a library which contains current objectFile
        std::shared_ptr<KleeFilesInfo> kleeFilesInfo; // Information about klee files

        // Directory from where to execute the command
        [[nodiscard]] fs::path const &getDirectory() const;
        // User source file
        [[nodiscard]] fs::path getSourcePath() const;
        // User object file
        [[nodiscard]] fs::path getOutputFile() const;

        void setOutputFile(const fs::path &file);

        void addFile(fs::path file);
    };

    /*
    * struct TargetInfo represents
    * a link command from link_commands.json
    * OR
    * a compile command from compile_commands.json that produces an executable
    */
    struct TargetInfo {
        //Linkage command
        std::vector<utbot::LinkCommand> commands;

        // Source files, object files and libraries that current command depends on
        tsl::ordered_set<fs::path, HashUtils::PathHash> files;

        // Units which contains current library
        std::vector <fs::path> parentLinkUnits;

        void addFile(fs::path file);

        // Executable or a library, the result of a command
        [[nodiscard]] fs::path getOutput() const;
    };

public:
    BuildDatabase(const fs::path &buildCommandsJsonPath,
                  fs::path serverBuildDir,
                  utbot::ProjectContext projectContext);

    static std::shared_ptr<BuildDatabase> create(const utbot::ProjectContext &projectContext);

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
     * @brief Returns compile command information for current source file or object file
     *
     * @param filepath Path to source file or object file
     * @return ObjectFileInfo for current source file or object file
     * @throws CompilationDatabaseException if files are wrong
     */
    [[nodiscard]] std::shared_ptr<const ObjectFileInfo> getClientCompilationUnitInfo(const fs::path &filepath) const;

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

    /**
     * @brief Gets all stub files associated with given link unit
     *
     * @param linkUnitInfo link unit info (preferably library)
     *
     * @return set of file paths to stubs
     */
    CollectionUtils::FileSet
    getStubFiles(const std::shared_ptr<const BuildDatabase::TargetInfo> &linkUnitInfo) const;

    /**
     * @brief Assign set of file paths to stubs to given link unit
     *
     * @param linkUnitInfo link unit info (preferably library)
     * @param stubs set of file paths to stubs
     */
    void assignStubFilesToLinkUnit(
        std::shared_ptr<const BuildDatabase::TargetInfo> linkUnitInfo,
        CollectionUtils::FileSet stubs);

    std::vector<std::shared_ptr<TargetInfo>> getRootTargets() const;
    std::vector<std::shared_ptr<TargetInfo>> getAllTargets() const;

    std::vector<fs::path>
    autoTargetListForFile(const fs::path &sourceFilePath, const fs::path &objectFile) const;

    std::vector<std::shared_ptr<TargetInfo>>
    getTargetsForSourceFile(fs::path const&sourceFilePath) const;

    std::shared_ptr<TargetInfo> getPriorityTarget() const;
private:
    fs::path serverBuildDir;
    utbot::ProjectContext projectContext;
    fs::path linkCommandsJsonPath;
    fs::path compileCommandsJsonPath;
    CollectionUtils::MapFileTo<std::vector<std::shared_ptr<ObjectFileInfo>>> sourceFileInfos;
    CollectionUtils::MapFileTo<std::shared_ptr<ObjectFileInfo>> objectFileInfos;
    CollectionUtils::MapFileTo<std::shared_ptr<TargetInfo>> targetInfos;
    CollectionUtils::MapFileTo<std::vector<fs::path>> objectFileTargets;

    std::unordered_map<std::shared_ptr<const BuildDatabase::TargetInfo>, CollectionUtils::FileSet>
        linkUnitToStubFiles;

    static bool conflictPriorityMore(const std::shared_ptr<ObjectFileInfo> &left,
                                     const std::shared_ptr<ObjectFileInfo> &right);

    void addLocalSharedLibraries();
    void fillTargetInfoParents();
    static fs::path getCorrespondingBitcodeFile(const fs::path &filepath);
    void createClangCompileCommandsJson(const fs::path &buildCommandsJsonPath,
                                        const nlohmann::json &compileCommandsJson);
    void initInfo(const nlohmann::json &linkCommandsJson);
    void mergeLibraryOptions(std::vector<std::string> &jsonArguments) const;
    fs::path newDirForFile(fs::path const& file) const;
    fs::path
    createExplicitObjectFileCompilationCommand(const std::shared_ptr<ObjectFileInfo> &objectInfo);

    using sharedLibrariesMap = std::unordered_map<std::string, CollectionUtils::MapFileTo<fs::path>>;

    template <typename Info>
    void addLibrariesForCommand(utbot::BaseCommand *command,
                                const std::shared_ptr<Info> &info,
                                sharedLibrariesMap &sharedLibraryFiles,
                                bool objectFiles = false);
};

#endif //UNITTESTBOT_BUILDDATABASE_H
