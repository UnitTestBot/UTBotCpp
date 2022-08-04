#include "BuildDatabase.h"

#include "BaseCommand.h"
#include "FeaturesFilter.h"
#include "Paths.h"
#include "building/CompileCommand.h"
#include "exceptions/CompilationDatabaseException.h"
#include "utils/DynamicLibraryUtils.h"
#include "utils/JsonUtils.h"
#include "utils/StringUtils.h"
#include "utils/GrpcUtils.h"
#include "utils/GenerationUtils.h"

#include "loguru.h"

#include <functional>
#include <queue>
#include <unordered_map>

static std::string tryConvertOptionToPath(const std::string &possibleFilePath,
                                          const fs::path &dirPath) {
    if (StringUtils::startsWith(possibleFilePath, "-")) {
        return possibleFilePath;
    }
    fs::path fullFilePath;
    try {
        fullFilePath = Paths::getCCJsonFileFullPath(possibleFilePath, dirPath);
    } catch (...) {
        return possibleFilePath;
    }
    return fs::exists(fullFilePath) ? fullFilePath.string() : possibleFilePath;
}

BuildDatabase::BuildDatabase(fs::path _buildCommandsJsonPath,
                             fs::path _serverBuildDir,
                             utbot::ProjectContext _projectContext,
                             bool createClangCC) :
        serverBuildDir(std::move(_serverBuildDir)),
        projectContext(std::move(_projectContext)),
        buildCommandsJsonPath(std::move(_buildCommandsJsonPath)),
        linkCommandsJsonPath(fs::canonical(buildCommandsJsonPath / "link_commands.json")),
        compileCommandsJsonPath(fs::canonical(buildCommandsJsonPath / "compile_commands.json")) {
    if (!fs::exists(linkCommandsJsonPath) || !fs::exists(compileCommandsJsonPath)) {
        throw CompilationDatabaseException("Couldn't open link_commands.json or compile_commands.json files");
    }

    auto linkCommandsJson = JsonUtils::getJsonFromFile(linkCommandsJsonPath);
    auto compileCommandsJson = JsonUtils::getJsonFromFile(compileCommandsJsonPath);

    initObjects(compileCommandsJson);
    initInfo(linkCommandsJson);
    filterInstalledFiles();
    addLocalSharedLibraries();
    fillTargetInfoParents();
    if (createClangCC) {
        createClangCompileCommandsJson();
    }
    target = GrpcUtils::UTBOT_AUTO_TARGET_PATH;
}

BuildDatabase::BuildDatabase(BuildDatabase& baseBuildDatabase,
                             const std::string &_target) :
        serverBuildDir(baseBuildDatabase.serverBuildDir),
        projectContext(baseBuildDatabase.projectContext),
        buildCommandsJsonPath(baseBuildDatabase.buildCommandsJsonPath),
        linkCommandsJsonPath(baseBuildDatabase.linkCommandsJsonPath),
        compileCommandsJsonPath(baseBuildDatabase.compileCommandsJsonPath) {

//    BuildDatabase baseBuildDatabase(buildCommandsJsonPath, serverBuildDir, projectContext, false);


//    fs::path target;
    //TODO target incorrect name now
    if (Paths::isSourceFile(_target)) {
        fs::path root = baseBuildDatabase.getRootForSource(_target);
        target = root;
    } else if (_target == GrpcUtils::UTBOT_AUTO_TARGET_PATH || _target.empty()) {
        fs::path root = baseBuildDatabase.getRootForFirstSource();
        target = root;
    } else {
        auto new_target = GenerationUtils::findTarget(baseBuildDatabase.getAllTargets(), _target);
        if (new_target.has_value()) {
            target = new_target.value();
        } else {
            throw CompilationDatabaseException("Can't find target: " + _target);
        }
    }


//    CollectionUtils::MapFileTo<std::vector<std::shared_ptr<ObjectFileInfo>>> sourceFileInfos;
//    CollectionUtils::MapFileTo<std::shared_ptr<ObjectFileInfo>> objectFileInfos;
//    CollectionUtils::MapFileTo<std::vector<fs::path>> objectFileTargets;
    {
        auto objectFilesList = baseBuildDatabase.getArchiveObjectFiles(target);
        for (const auto &objectFilePath: objectFilesList) {
            auto objectFileInfo = baseBuildDatabase.getClientCompilationObjectInfo(objectFilePath);
            sourceFileInfos[objectFileInfo->getSourcePath()].push_back(objectFileInfo);
            LOG_IF_S(DEBUG, sourceFileInfos[objectFileInfo->getSourcePath()].size() > 1)
            << "Multiple compile commands for file \"" << objectFileInfo->getSourcePath() << "\" in target \""
            << target.string() << "\"";
            objectFileInfos[objectFileInfo->getOutputFile()] = objectFileInfo;
            objectFileTargets[objectFileInfo->getOutputFile()] =
                    baseBuildDatabase.objectFileTargets[objectFileInfo->getOutputFile()];
        }
    }
//    CollectionUtils::MapFileTo<std::shared_ptr<TargetInfo>> targetInfos;
//    CollectionUtils::MapFileTo<CollectionUtils::FileSet> linkUnitToStubFiles;
    {
        auto targetFilesList = baseBuildDatabase.getArchiveTargetFiles(target);
        for (const auto &objectFilePath: targetFilesList) {
            targetInfos[objectFilePath] = baseBuildDatabase.targetInfos[objectFilePath];
            linkUnitToStubFiles[objectFilePath] = baseBuildDatabase.linkUnitToStubFiles[objectFilePath];
        }
    }

//    std::vector<std::pair<nlohmann::json, std::shared_ptr<ObjectFileInfo>>> compileCommands_temp;
    compileCommands_temp = baseBuildDatabase.compileCommands_temp;

    createClangCompileCommandsJson();
}

std::shared_ptr<BuildDatabase> BuildDatabase::createBaseForTarget(const std::string &_target) {
    return std::make_shared<BuildDatabase>(std::move(BuildDatabase(*this, _target)));
}

std::shared_ptr<BuildDatabase> BuildDatabase::create(const utbot::ProjectContext &projectContext) {
    fs::path compileCommandsJsonPath =
            CompilationUtils::substituteRemotePathToCompileCommandsJsonPath(
                    projectContext.projectPath, projectContext.buildDirRelativePath);
    fs::path serverBuildDir = Paths::getUtbotBuildDir(projectContext);
    std::shared_ptr<BuildDatabase> buildDatabase = std::make_shared<BuildDatabase>(compileCommandsJsonPath, serverBuildDir, projectContext, true);
    return buildDatabase;
}

fs::path BuildDatabase::createExplicitObjectFileCompilationCommand(const std::shared_ptr<ObjectFileInfo> &objectInfo) {
    if (Paths::isSourceFile(objectInfo->getSourcePath())) {
        auto outputFile = objectInfo->getOutputFile();
        auto tmpObjectFileName =
            Paths::createTemporaryObjectFile(outputFile, objectInfo->getSourcePath());
        objectInfo->setOutputFile(tmpObjectFileName);
        // redirect existing compilation command to temporary file
        LOG_IF_S(ERROR, CollectionUtils::containsKey(objectFileInfos, tmpObjectFileName))
            << "Temporary object file name generated by UTBot is already present in the "
               "project: "
            << tmpObjectFileName;
        objectInfo->linkUnit = outputFile;
        objectFileInfos[tmpObjectFileName] = objectInfo;
        return tmpObjectFileName;
    } else {
        return objectInfo->getSourcePath();
    }
}

void BuildDatabase::initObjects(const nlohmann::json &compileCommandsJson) {
    for (const nlohmann::json &compileCommand: compileCommandsJson) {
        auto objectInfo = std::make_shared<ObjectFileInfo>();

        fs::path directory = compileCommand.at("directory").get<std::string>();
        fs::path jsonFile = compileCommand.at("file").get<std::string>();
        fs::path sourceFile = Paths::getCCJsonFileFullPath(jsonFile, directory);

        std::vector<std::string> jsonArguments;
        if (compileCommand.contains("command")) {
            std::string command = compileCommand.at("command");
            jsonArguments = StringUtils::splitByWhitespaces(command);
        } else {
            jsonArguments = std::vector<std::string>(compileCommand.at("arguments"));
        }
        std::transform(jsonArguments.begin(), jsonArguments.end(), jsonArguments.begin(),
                       [&directory](const std::string &argument) {
                           return tryConvertOptionToPath(argument, directory);
                       });
        objectInfo->command = utbot::CompileCommand(jsonArguments, directory, sourceFile);
        objectInfo->command.removeWerror();
        fs::path outputFile = objectInfo->getOutputFile();
        fs::path kleeFilePathTemplate =
            Paths::createNewDirForFile(sourceFile, projectContext.buildDir(), serverBuildDir);
        fs::path kleeFile = Paths::addSuffix(kleeFilePathTemplate, "_klee");
        objectInfo->kleeFilesInfo = std::make_shared<KleeFilesInfo>(kleeFile);

        if (CollectionUtils::containsKey(objectFileInfos, outputFile) ||
            CollectionUtils::containsKey(targetInfos, outputFile)) {
            /*
             * If the condition above is true, that means that the output file
             * is built from multiple sources. Hence, it is not an object file,
             * but an executable, and it should be treated as a target.
             * This is a hack. This inconsistency is produced by Bear
             * when it treats a Makefile command like
             * gcc -o output a.c b.c c.c
             * This code is creating artificial compile and link commands, similar
             * to commands Bear generates from CMake command like
             * add_executable(output a.c b.c c.c)
             */
            auto targetInfo = targetInfos[outputFile];
            if (targetInfo == nullptr) {
                LOG_S(DEBUG) << outputFile << " is treated as a target instead of an object file";
                auto targetObjectInfo = objectFileInfos[outputFile];
                auto tmpObjectFileName = createExplicitObjectFileCompilationCommand(targetObjectInfo);
                objectFileInfos.erase(outputFile);

                //create targetInfo
                targetInfo = targetInfos[outputFile] = std::make_shared<TargetInfo>();
                targetInfo->commands.emplace_back(
                        std::initializer_list<std::string>{targetObjectInfo->command.getCompiler(),
                                                           "-o", outputFile, tmpObjectFileName},
                        directory);
                targetInfo->addFile(tmpObjectFileName);
            }
            //redirect new compilation command to temporary file
            auto tmpObjectFileName = createExplicitObjectFileCompilationCommand(objectInfo);

            //add new dependency to an implicit target
            targetInfo->commands[0].addFlagToEnd(tmpObjectFileName);
            targetInfo->addFile(tmpObjectFileName);
        } else {
            objectFileInfos[outputFile] = objectInfo;
        }
        compileCommands_temp.emplace_back(compileCommand, objectInfo);
        const fs::path &sourcePath = objectInfo->getSourcePath();
        sourceFileInfos[sourcePath].emplace_back(objectInfo);
    }
    for (auto &[sourceFile, objectInfos]: sourceFileInfos) {
        //Need stable sort for save order of 32 and 64 bits files
        std::stable_sort(objectInfos.begin(), objectInfos.end(), conflictPriorityMore);
    }
}

void BuildDatabase::initInfo(const nlohmann::json &linkCommandsJson) {
    for (nlohmann::json const &linkCommand : linkCommandsJson) {
        fs::path directory = linkCommand.at("directory").get<std::string>();
        std::vector<std::string> jsonArguments;
        if (linkCommand.contains("command")) {
            std::string command = linkCommand.at("command");
            jsonArguments = StringUtils::splitByWhitespaces(command);
        } else {
            jsonArguments = std::vector<std::string>(linkCommand.at("arguments"));
        }
        if (StringUtils::endsWith(jsonArguments[0], "ranlib") ||
            StringUtils::endsWith(jsonArguments[0], "cmake")) {
            continue;
        }
        std::transform(jsonArguments.begin(), jsonArguments.end(), jsonArguments.begin(),
                       [&directory](const std::string &argument) {
                         return tryConvertOptionToPath(argument, directory);
                       });

        mergeLibraryOptions(jsonArguments);

        utbot::LinkCommand command(jsonArguments, directory);
        fs::path const &output = command.getOutput();
        auto targetInfo = targetInfos[output];
        if (targetInfo == nullptr) {
            targetInfo = targetInfos[output] = std::make_shared<TargetInfo>();
        } else {
            LOG_S(WARNING) << "Multiple commands for one file: " << output.string();
        }
        for (nlohmann::json const &jsonFile: linkCommand.at("files")) {
            auto filename = jsonFile.get<std::string>();
            fs::path currentFile = Paths::getCCJsonFileFullPath(filename, command.getDirectory());
            targetInfo->addFile(currentFile);
            if (Paths::isObjectFile(currentFile)) {
                if (!CollectionUtils::containsKey(objectFileInfos, currentFile)) {
                    throw CompilationDatabaseException("compile_commands.json doesn't contain a command for object file "
                                                       + currentFile.string());
                }
                objectFileInfos[currentFile]->linkUnit = output;
            }
        }
        targetInfo->commands.emplace_back(command);
    }
}

void BuildDatabase::createClangCompileCommandsJson() {
    CollectionUtils::MapFileTo<std::pair<nlohmann::json, std::shared_ptr<ObjectFileInfo>>> fileCompileCommands;
    for (const auto &[compileCommand, objectInfo]: compileCommands_temp) {
        const fs::path &sourcePath = objectInfo->getSourcePath();
        if (CollectionUtils::contains(fileCompileCommands, sourcePath)) {
            LOG_S(WARNING) << "Multiple compile commands for file \"" << sourcePath
                            << "\" use command for \"" << objectInfo->getOutputFile() << "\"";
        } else if (CollectionUtils::contains(objectFileInfos, objectInfo->getOutputFile())) {
            fileCompileCommands[sourcePath] = {compileCommand, objectInfo};
        }
    }

    nlohmann::json compileCommandsSingleFilesJson;
    for (const auto &compileCommand: fileCompileCommands) {
        compileCommandsSingleFilesJson.push_back(compileCommand.second.first);
    }

    fs::path clangCompileCommandsJsonPath = CompilationUtils::getClangCompileCommandsJsonPath(buildCommandsJsonPath);
    JsonUtils::writeJsonToFile(clangCompileCommandsJsonPath, compileCommandsSingleFilesJson);
    compilationDatabase = CompilationUtils::getCompilationDatabase(compileCommandsJsonPath);
}

//void BuildDatabase::updateTarget(const fs::path &_target) {
//    if (_target.string() == GrpcUtils::UTBOT_AUTO_TARGET_PATH) {
//        return;
//    }
//
//    for (auto &[sourceFile, objectInfos]: sourceFileInfos) {
//        std::sort(objectInfos.begin(), objectInfos.end(), [&](const std::shared_ptr<ObjectFileInfo> &left,
//                                                              const std::shared_ptr<ObjectFileInfo> &right) {
//            if (CollectionUtils::containsKey(targetInfos, target)) {
//                if (CollectionUtils::containsKey(targetInfos[target]->files, left->getOutputFile())) {
//                    return true;
//                }
//                if (CollectionUtils::containsKey(targetInfos[target]->files, right->getOutputFile())) {
//                    return false;
//                }
//            }
//            return false;
//        });
//    }
//}

void BuildDatabase::mergeLibraryOptions(std::vector<std::string> &jsonArguments) const {
    for (auto it = jsonArguments.begin(); it != jsonArguments.end(); it++) {
        if (*it == DynamicLibraryUtils::libraryDirOption || *it == DynamicLibraryUtils::linkFlag) {
            auto next = std::next(it);
            *it += *next;
            *next = "";
        }
    }
    CollectionUtils::erase(jsonArguments, "");
}

namespace {
    CollectionUtils::OrderedFileSet collectLibraryDirs(const utbot::BaseCommand &command) {
        using namespace DynamicLibraryUtils;
        CollectionUtils::OrderedFileSet libraryDirs;
        for (std::string const &argument : command.getCommandLine()) {
            auto optionalLibraryPath = getLibraryAbsolutePath(argument, command.getDirectory());
            if (optionalLibraryPath.has_value()) {
                libraryDirs.insert(optionalLibraryPath.value());
            }
            if (StringUtils::startsWith(argument, libraryDirOptionWl)) {
                auto commaSeparated = StringUtils::split(argument, ',');
                bool isRpathNext = false;
                for (auto part : commaSeparated) {
                    if (part == rpathFlag) {
                        isRpathNext = true;
                        continue;
                    }
                    if (isRpathNext) {
                        isRpathNext = false;
                        libraryDirs.insert(part);
                    }
                }
            }
        }
        return libraryDirs;
    }

    CollectionUtils::MapFileTo<std::string> collectLibraryNames(const utbot::BaseCommand &command) {
        using namespace DynamicLibraryUtils;

        CollectionUtils::MapFileTo<std::string> libraryNames;

        for (const auto &argument : command.getCommandLine()) {
            if (Paths::isSharedLibraryFile(argument) && argument != command.getOutput() &&
                !StringUtils::startsWith(argument, libraryDirOptionWl)) {
                libraryNames.emplace(argument, argument);
            }
            if (StringUtils::startsWith(argument, linkFlag)) {
                std::string libraryName = argument.substr(linkFlag.length());
                std::string archiveFile = "lib" + libraryName + ".a";
                std::string sharedObjectFile = "lib" + libraryName + ".so";
                libraryNames.emplace(sharedObjectFile, argument);
                libraryNames.emplace(archiveFile, argument);
            }
        }
        return libraryNames;
    }
}

void BuildDatabase::addLibrariesForCommand(utbot::BaseCommand &command,
                                           BaseFileInfo &info,
                                           sharedLibrariesMap &sharedLibraryFiles,
                                           bool objectFiles) {
    if (command.isArchiveCommand()) {
        return;
    }
    auto libraryDirs = collectLibraryDirs(command);
    auto libraryNames = collectLibraryNames(command);
    std::unordered_map<std::string, fs::path> argumentToFile;
    for (auto const &[libraryName, argument] : libraryNames) {
        fs::path name = libraryName;
        for (auto const &libraryDir : libraryDirs) {
            if (CollectionUtils::containsKey(sharedLibraryFiles, name)) {
                if (CollectionUtils::containsKey(sharedLibraryFiles.at(name), libraryDir)) {
                    name = sharedLibraryFiles.at(name).at(libraryDir);
                }
            }
            fs::path fullPath = Paths::getCCJsonFileFullPath(name, libraryDir);
            if (CollectionUtils::containsKey(targetInfos, fullPath)) {
                info.addFile(fullPath);
                LOG_IF_S(WARNING, objectFiles) << "Object file " << command.getOutput()
                                               << " has library dependencies: " << fullPath;
                argumentToFile[argument] = fullPath;
            } else {
                info.installedFiles.insert(fullPath);
            }
        }
    }
    for (auto &argument : command.getCommandLine()) {
        if (CollectionUtils::containsKey(argumentToFile, argument)) {
            argument = argumentToFile[argument];
        }
    }
}

void BuildDatabase::filterInstalledFiles() {
    for (auto &it : targetInfos) {
        auto &linkFile = it.first;
        auto &targetInfo = it.second;
        CollectionUtils::OrderedFileSet fileset;
        targetInfo->installedFiles =
            CollectionUtils::filterOut(targetInfo->files, [this](fs::path const &file) {
                return CollectionUtils::containsKey(targetInfos, file) ||
                       CollectionUtils::containsKey(objectFileInfos, file);
            });
        if (!targetInfo->installedFiles.empty()) {
            LOG_S(DEBUG) << "Target " << linkFile << " depends on " << targetInfo->installedFiles.size() << " installed files";
        }
        CollectionUtils::erase_if(targetInfo->files, [&targetInfo](fs::path const &file) {
            return CollectionUtils::contains(targetInfo->installedFiles, file);
        });
    }
}

void BuildDatabase::addLocalSharedLibraries() {
    sharedLibrariesMap sharedLibraryFiles;
    for (const auto &[linkFile, linkUnit] : targetInfos) {
        if (Paths::isSharedLibraryFile(linkFile)) {
            auto withoutVersion = CompilationUtils::removeSharedLibraryVersion(linkFile);
            sharedLibraryFiles[withoutVersion.filename()][linkFile.parent_path()] = linkFile;
        }
    }
    for (auto &[linkFile, targetInfo] : targetInfos) {
        for (auto &command : targetInfo->commands) {
            addLibrariesForCommand(command, *targetInfo, sharedLibraryFiles);
        }
    }
    for (auto &[objectFile, objectInfo] : objectFileInfos) {
        addLibrariesForCommand(objectInfo->command, *objectInfo, sharedLibraryFiles, true);
    }
}

void BuildDatabase::fillTargetInfoParents() {
    CollectionUtils::MapFileTo<std::vector<fs::path>> parentTargets;
    for (const auto &[linkFile, linkUnit] : targetInfos) {
        for (const fs::path &dependencyFile : linkUnit->files) {
            if (Paths::isLibraryFile(dependencyFile)) {
                parentTargets[dependencyFile].emplace_back(linkFile);
            }
            if (Paths::isObjectFile(dependencyFile)) {
                objectFileTargets[dependencyFile].emplace_back(linkFile);
            }
        }
    }
    for (auto &[library, parents] : parentTargets) {
        if (!CollectionUtils::containsKey(targetInfos, library)) {
            throw CompilationDatabaseException(
                "link_commands.json doesn't contain a command for building library: " +
                library.string() + "\nReferenced from command for: " + (parents.empty() ? "none" : parents[0].string()));
        }
        targetInfos[library]->parentLinkUnits = std::move(parents);
    }
}

const fs::path &BuildDatabase::getCompileCommandsJson() {
    return compileCommandsJsonPath;
}

const fs::path &BuildDatabase::getLinkCommandsJson() {
    return linkCommandsJsonPath;
}

std::vector<std::shared_ptr<BuildDatabase::ObjectFileInfo>>
BuildDatabase::getAllCompileCommands() const {
    std::vector<std::shared_ptr<ObjectFileInfo>> result;
    for (auto &[file, compilationUnit] : objectFileInfos) {
        result.emplace_back(compilationUnit);
    }
    return result;
}

fs::path BuildDatabase::getObjectFile(const fs::path &sourceFile) const {
    if (!CollectionUtils::containsKey(sourceFileInfos, sourceFile)) {
        throw CompilationDatabaseException("Couldn't find object file for current source file " +
                                           sourceFile.string());
    }
    auto objectInfo = sourceFileInfos.at(sourceFile)[0];
    return objectInfo->getOutputFile();
}

CollectionUtils::FileSet BuildDatabase::getArchiveObjectFiles(const fs::path &archive) const {
    if (Paths::isGtest(archive)) {
        return {};
    }
    if (!CollectionUtils::containsKey(targetInfos, archive)) {
        throw CompilationDatabaseException(
            "Couldn't find current archive file linkage information for " + archive.string());
    }
    std::shared_ptr<TargetInfo> targetInfo = targetInfos.at(archive);
    CollectionUtils::FileSet result;
    for (const auto &file : targetInfo->files) {
        if (Paths::isLibraryFile(file)) {
            auto archiveObjectFiles = getArchiveObjectFiles(file);
            CollectionUtils::extend(result, archiveObjectFiles);
        } else {
            const fs::path &sourcePath = getClientCompilationObjectInfo(file)->getSourcePath();
            if (Paths::isSourceFile(sourcePath)) {
                result.insert(file);
            } else {
                LOG_S(WARNING) << "Skipping not c/c++ source file: " << sourcePath.string();
            }
        }
    }
    return result;
}

CollectionUtils::FileSet BuildDatabase::getArchiveTargetFiles(const fs::path &archive) const {
    if (Paths::isGtest(archive)) {
        return {};
    }
    if (!CollectionUtils::containsKey(targetInfos, archive)) {
        throw CompilationDatabaseException(
            "Couldn't find current archive file linkage information for " + archive.string());
    }
    std::shared_ptr<TargetInfo> targetInfo = targetInfos.at(archive);
    CollectionUtils::FileSet result = {archive};
    for (const auto &file: targetInfo->files) {
        if (Paths::isLibraryFile(file)) {
            auto archiveObjectFiles = getArchiveTargetFiles(file);
            result.insert(file);
            CollectionUtils::extend(result, archiveObjectFiles);
        }
    }
    return result;
}

fs::path BuildDatabase::getRootForSource(const fs::path& path) const {
    fs::path normalizedPath = Paths::normalizedTrimmed(path);
    if (Paths::isSourceFile(normalizedPath)) {
        if (!CollectionUtils::containsKey(sourceFileInfos, normalizedPath)) {
            throw CompilationDatabaseException("No executable or library found for current source file in link_commands.json: " + path.string());
        }
        auto const &sourceFileInfo = sourceFileInfos.at(normalizedPath);

        auto linkUnit = sourceFileInfo[0]->linkUnit;
        if (linkUnit.empty()) {
            throw CompilationDatabaseException("No executable or library found for current source file in link_commands.json: " + path.string());
        }
        return getRootForSource(linkUnit);
    } else {
        if (!CollectionUtils::containsKey(targetInfos, normalizedPath)) {
            throw CompilationDatabaseException("No executable or library found for current library in link_commands.json: " + path.string());
        }
        auto linkUnit = targetInfos.at(normalizedPath);
        if (!linkUnit->parentLinkUnits.empty()) {
            return getRootForSource(linkUnit->parentLinkUnits[0]);
        } else {
            return linkUnit->getOutput();
        }
    }
}


fs::path BuildDatabase::getRootForFirstSource() const {
    return getRootForSource(sourceFileInfos.begin()->first);
}

fs::path BuildDatabase::getBitcodeForSource(const fs::path &sourceFile) const {
    fs::path serverBuildObjectFilePath = newDirForFile(sourceFile);
    return Paths::addExtension(serverBuildObjectFilePath, ".bc");
}

fs::path BuildDatabase::getBitcodeFile(const fs::path &filepath) const {
    auto objectInfo = sourceFileInfos.find(filepath);
    if (objectInfo != sourceFileInfos.end()) {
        return getBitcodeForSource(objectInfo->second[0]->getSourcePath());
    } else {
        auto objectInfo = objectFileInfos.find(filepath);
        if (objectInfo != objectFileInfos.end()) {
            return getBitcodeForSource(objectInfo->second->getSourcePath());
        } else {
            auto targetInfo = targetInfos.find(filepath);
            if (targetInfo != targetInfos.end()) {
                fs::path movedFile = newDirForFile(filepath);
                return getCorrespondingBitcodeFile(movedFile);
            }
            return getCorrespondingBitcodeFile(filepath);
        }
    }
}


std::shared_ptr<BuildDatabase::ObjectFileInfo> BuildDatabase::getClientCompilationObjectInfo(const fs::path &filepath) const {
    if (!CollectionUtils::contains(objectFileInfos, filepath)) {
        throw CompilationDatabaseException("File not found in compilation_commands.json: " + filepath.string());
    }
    return objectFileInfos.at(filepath);
}

std::shared_ptr<BuildDatabase::ObjectFileInfo> BuildDatabase::getClientCompilationSourceInfo(const fs::path &filepath) const {
    if (!CollectionUtils::contains(sourceFileInfos, filepath)) {
        throw CompilationDatabaseException("File not found in compilation_commands.json: " + filepath.string());
    }
    LOG_IF_S(DEBUG, sourceFileInfos.at(filepath).size() > 1) << "More than one compile command for: " << filepath;
    return sourceFileInfos.at(filepath)[0];
}

std::shared_ptr<const BuildDatabase::ObjectFileInfo> BuildDatabase::getClientCompilationUnitInfo(const fs::path &filepath) const {
    if (Paths::isSourceFile(filepath)) {
        return getClientCompilationSourceInfo(filepath);
    }
    if (Paths::isObjectFile(filepath)) {
        return getClientCompilationObjectInfo(filepath);
    }
    throw CompilationDatabaseException("File is not a compilation unit or an object file: " + filepath.string());
}

std::shared_ptr<const BuildDatabase::TargetInfo> BuildDatabase::getClientLinkUnitInfo(const fs::path &filepath) const {
    if (Paths::isSourceFile(filepath)) {
        auto compilationInfo = getClientCompilationUnitInfo(filepath);
        return targetInfos.at(compilationInfo->linkUnit);
    }
    if (CollectionUtils::containsKey(targetInfos, filepath)) {
        return targetInfos.at(filepath);
    }
    throw CompilationDatabaseException("File is not in link_commands.json: " +
                                       filepath.string());
}

bool BuildDatabase::conflictPriorityMore(
        const std::shared_ptr<BuildDatabase::ObjectFileInfo> &left,
        const std::shared_ptr<BuildDatabase::ObjectFileInfo> &right) {
    if (StringUtils::contains(left->getOutputFile().string(), "64")) {
        return true;
    }
    if (StringUtils::contains(right->getOutputFile().string(), "64")) {
        return false;
    }
    return false;
}

fs::path BuildDatabase::getCorrespondingBitcodeFile(const fs::path &filepath) {
    return Paths::replaceExtension(filepath, ".bc");
}

bool BuildDatabase::isFirstObjectFileForSource(const fs::path &objectFilePath) const {
    fs::path sourceFile = getClientCompilationUnitInfo(objectFilePath)->getSourcePath();
    fs::path firstObjectFileForSource = getClientCompilationUnitInfo(sourceFile)->getOutputFile();
    return objectFilePath == firstObjectFileForSource;
}

BuildDatabase::KleeFilesInfo::KleeFilesInfo(fs::path kleeFile) : kleeFile(std::move(kleeFile)) {
    fs::create_directories(this->kleeFile.parent_path());
}

void BuildDatabase::KleeFilesInfo::setCorrectMethods(std::unordered_set<std::string> correctMethods) {
    this->correctMethods = std::move(correctMethods);
}

bool BuildDatabase::KleeFilesInfo::isCorrectMethod(const std::string &method) {
    if (allAreCorrect) {
        return true;
    }
    return CollectionUtils::contains(correctMethods, method);
}

fs::path BuildDatabase::KleeFilesInfo::getKleeFile() {
    return getKleeFile("");
}

fs::path BuildDatabase::KleeFilesInfo::getKleeBitcodeFile() {
    return getKleeBitcodeFile("");
}

fs::path BuildDatabase::KleeFilesInfo::getKleeFile(const std::string& methodName) {
    return kleeFile;
}

fs::path BuildDatabase::KleeFilesInfo::getKleeBitcodeFile(const std::string& methodName) {
    return getCorrespondingBitcodeFile(getKleeFile());
}

void BuildDatabase::KleeFilesInfo::setAllAreCorrect(bool allAreCorrect) {
    this->allAreCorrect = allAreCorrect;
}

fs::path const &BuildDatabase::ObjectFileInfo::getDirectory() const {
    return command.getDirectory();
}

fs::path BuildDatabase::ObjectFileInfo::getSourcePath() const {
    return command.getSourcePath();
}

fs::path BuildDatabase::ObjectFileInfo::getOutputFile() const {
    return command.getOutput();
}

void BuildDatabase::ObjectFileInfo::setOutputFile(const fs::path &file) {
    command.setOutput(file);
}

void BuildDatabase::BaseFileInfo::addFile(fs::path file) {
    files.insert(std::move(file));
}

bool BuildDatabase::ObjectFileInfo::is32bits() const {
    return CollectionUtils::contains(command.getCommandLine(), "-m32");
}

void BuildDatabase::TargetInfo::addFile(fs::path file) {
    files.insert(std::move(file));
}

fs::path BuildDatabase::TargetInfo::getOutput() const {
    return commands[0].getOutput();
}

CollectionUtils::FileSet BuildDatabase::getStubFiles(
    const std::shared_ptr<const BuildDatabase::TargetInfo> &linkUnitInfo) const {
    auto iterator = linkUnitToStubFiles.find(linkUnitInfo->getOutput());
    if (iterator != linkUnitToStubFiles.end()) {
        return iterator->second;
    }
    return {};
}

void BuildDatabase::assignStubFilesToLinkUnit(
    std::shared_ptr<const BuildDatabase::TargetInfo> linkUnitInfo,
    CollectionUtils::FileSet stubs) {
    linkUnitToStubFiles.emplace(linkUnitInfo->getOutput(), std::move(stubs));
}

std::vector<std::shared_ptr<BuildDatabase::TargetInfo>> BuildDatabase::getRootTargets() const {
    return CollectionUtils::filterOut(
            CollectionUtils::getValues(targetInfos),
            [](const std::shared_ptr<const BuildDatabase::TargetInfo> &linkUnitInfo) {
                return !linkUnitInfo->parentLinkUnits.empty();
            });
}

std::vector<std::shared_ptr<BuildDatabase::TargetInfo>> BuildDatabase::getAllTargets() const {
    return CollectionUtils::getValues(targetInfos);
}

std::vector<std::shared_ptr<BuildDatabase::TargetInfo>>
BuildDatabase::getTargetsForSourceFile(const fs::path &sourceFilePath) const {
    CollectionUtils::MapFileTo<bool> cache;
    std::function<bool(fs::path const &)> containsSourceFilePath = [&](fs::path const &unitFile) {
        if (CollectionUtils::containsKey(cache, unitFile)) {
            return cache[unitFile];
        }
        if (Paths::isObjectFile(unitFile)) {
            auto compilationUnitInfo = getClientCompilationUnitInfo(unitFile);
            bool isSame = compilationUnitInfo->getSourcePath() == sourceFilePath;
            return cache[unitFile] = isSame;
        }
        auto linkUnitInfo = getClientLinkUnitInfo(unitFile);
        bool result = CollectionUtils::anyTrue(CollectionUtils::transform(
            linkUnitInfo->files, [&containsSourceFilePath](fs::path const &subFile) {
                return containsSourceFilePath(subFile);
            }));
        return cache[unitFile] = result;
    };

    auto rootTargets = getRootTargets();
    return CollectionUtils::filterOut(
            rootTargets, [&](const std::shared_ptr<const BuildDatabase::TargetInfo> &rootTarget) {
                return !containsSourceFilePath(rootTarget->getOutput());
            });
}

std::vector<fs::path> BuildDatabase::targetListForFile(const fs::path &sourceFilePath,
                                                       const fs::path &objectFile) const {
//    if (!hasAutoTarget()) {
//        return {target};
//    }
    auto result = CollectionUtils::transformTo<std::vector<fs::path>>(
            getTargetsForSourceFile(sourceFilePath),
            [&](const std::shared_ptr<const BuildDatabase::TargetInfo> &targetInfo) {
                return targetInfo->getOutput();
            });
    std::vector<fs::path> parents;
    if (CollectionUtils::containsKey(objectFileTargets, objectFile)) {
        parents = objectFileTargets.at(objectFile);
    } else {
        LOG_S(WARNING) << "No link unit parents were found for an object file: " << objectFile;
    }
    result.insert(
            result.end(),
            std::make_move_iterator(parents.begin()),
            std::make_move_iterator(parents.end())
    );
    return result;
}

std::shared_ptr<BuildDatabase::TargetInfo> BuildDatabase::getPriorityTarget() const {
    CollectionUtils::MapFileTo<int> cache;
    std::function<int(fs::path const &)> numberOfSources = [&](fs::path const &unitFile) {
        if (CollectionUtils::containsKey(cache, unitFile)) {
            return cache[unitFile];
        }
        if (Paths::isObjectFile(unitFile)) {
            return 1;
        }
        auto linkUnitInfo = getClientLinkUnitInfo(unitFile);
        int result = 0;
        for (const fs::path &subFile : linkUnitInfo->files) {
            result += numberOfSources(subFile);
        }
        return cache[unitFile] = result;
    };

    auto rootTargets = getRootTargets();
    auto it = std::max_element(rootTargets.begin(), rootTargets.end(),
                               [&](const std::shared_ptr<BuildDatabase::TargetInfo>& a,
                                   const std::shared_ptr<BuildDatabase::TargetInfo>& b) {
                                   return numberOfSources(a->getOutput()) <
                                          numberOfSources(b->getOutput());
                               });
    return *it;
}

fs::path BuildDatabase::newDirForFile(const fs::path &file) const {
    fs::path base = Paths::longestCommonPrefixPath(this->projectContext.buildDir(),
                                                   this->projectContext.projectPath);
    return Paths::createNewDirForFile(file, base, this->serverBuildDir);
}

CollectionUtils::FileSet BuildDatabase::getSourceFilesForTarget(const fs::path &_target) {
    LOG_IF_S(WARNING, !hasAutoTarget() && target != _target.c_str()) << "Try get sources for different target";
    return CollectionUtils::transformTo<CollectionUtils::FileSet>(
            getArchiveObjectFiles(_target),
            [this](fs::path const &objectPath) {
                return getClientCompilationUnitInfo(objectPath)->getSourcePath();
            });
}

bool BuildDatabase::hasAutoTarget() const {
    return target == GrpcUtils::UTBOT_AUTO_TARGET_PATH;
}

fs::path BuildDatabase::getTargetPath() const {
    return target;
}
