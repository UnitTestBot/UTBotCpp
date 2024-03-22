#include "ProjectBuildDatabase.h"

#include "utils/GrpcUtils.h"
#include "exceptions/CompilationDatabaseException.h"
#include "utils/JsonUtils.h"
#include "loguru.h"
#include "utils/StringUtils.h"
#include "utils/CompilationUtils.h"
#include "Paths.h"

static std::string tryConvertToFullPath(const std::string &possibleFilePath, const fs::path &dirPath) {
    fs::path fullFilePath = Paths::getFileFullPath(possibleFilePath, dirPath);
    return fs::exists(fullFilePath) ? fullFilePath.string() : possibleFilePath;
}

static std::string tryConvertOptionToPath(const std::string &possibleFilePath, const fs::path &dirPath) {
    std::string resOption;
    try {
        if (StringUtils::startsWith(possibleFilePath, "-I")) {
            resOption = CompilationUtils::getIncludePath(tryConvertToFullPath(possibleFilePath.substr(2), dirPath));
        } else if (!StringUtils::startsWith(possibleFilePath, "-")) {
            resOption = tryConvertToFullPath(possibleFilePath, dirPath);
        } else {
            resOption = possibleFilePath;
        }
    } catch (...) {
        return possibleFilePath;
    }
    return resOption;
}

ProjectBuildDatabase::ProjectBuildDatabase(fs::path _buildCommandsJsonPath,
                                           fs::path _serverBuildDir,
                                           utbot::ProjectContext _projectContext,
                                           bool skipObjectWithoutSource) :
        BuildDatabase(_serverBuildDir,
                      _buildCommandsJsonPath,
                      fs::canonical(_buildCommandsJsonPath / "link_commands.json"),
                      fs::canonical(_buildCommandsJsonPath / "compile_commands.json"),
                      std::move(_projectContext)) {
    if (!fs::exists(linkCommandsJsonPath) || !fs::exists(compileCommandsJsonPath)) {
        std::string message = "Couldn't open link_commands.json or compile_commands.json files";
        LOG_S(ERROR) << message;
        throw CompilationDatabaseException(message);
    }

    try {
        auto linkCommandsJson = JsonUtils::getJsonFromFile(linkCommandsJsonPath);
        auto compileCommandsJson = JsonUtils::getJsonFromFile(compileCommandsJsonPath);
        initObjects(compileCommandsJson);
        for (const auto i: sourceFileInfos) {
            LOG_S(MAX) << "Source: " << i.first << " Objects: "
                       << StringUtils::joinWith(CollectionUtils::transformTo<std::vector<std::string>>
                                                        (i.second, [](const std::shared_ptr<ObjectFileInfo> &s) {
                                                            return s->getOutputFile();
                                                        }), ", ");
        }
        initInfo(linkCommandsJson, skipObjectWithoutSource);
        for (const auto i: targetInfos) {
            LOG_S(MAX) << "Target: " << i.first << " Files: " << StringUtils::joinWith(i.second->files, ", ");
        }
        filterInstalledFiles();
        addLocalSharedLibraries();
        fillTargetInfoParents();
        createClangCompileCommandsJson();
    } catch (const std::exception &e) {
        LOG_S(ERROR) << e.what();
        return;
    }
}

ProjectBuildDatabase::ProjectBuildDatabase(utbot::ProjectContext projectContext, bool skipObjectWithoutSource)
        : ProjectBuildDatabase(
        CompilationUtils::substituteRemotePathToCompileCommandsJsonPath(projectContext),
        Paths::getUTBotBuildDir(projectContext), std::move(projectContext), skipObjectWithoutSource) {
}


void ProjectBuildDatabase::initObjects(const nlohmann::json &compileCommandsJson) {
    for (const nlohmann::json &compileCommand: compileCommandsJson) {
        auto objectInfo = std::make_shared<ObjectFileInfo>();

        fs::path directory = compileCommand.at("directory").get<std::string>();
        fs::path jsonFile = compileCommand.at("file").get<std::string>();
        fs::path sourceFile = Paths::getFileFullPath(jsonFile, directory);

        std::vector<std::string> jsonArguments;
        if (compileCommand.contains("command")) {
            std::string command = compileCommand.at("command");
            jsonArguments = StringUtils::splitByWhitespaces(command);
        } else {
            jsonArguments = std::vector<std::string>(compileCommand.at("arguments"));
        }
        LOG_S(MAX) << "Processing build command: " << StringUtils::joinWith(jsonArguments, " ");
        std::transform(jsonArguments.begin(), jsonArguments.end(), jsonArguments.begin(),
                       [&directory](const std::string &argument) {
                           return tryConvertOptionToPath(argument, directory);
                       });
        objectInfo->command = utbot::CompileCommand(jsonArguments, directory, sourceFile);
        objectInfo->command.removeWerror();
        fs::path outputFile = objectInfo->getOutputFile();

        if (!Paths::isSourceFile(sourceFile)) {
            LOG_S(INFO) << "Skip non C/C++ file: \"" << sourceFile << "\"";
            ignoredOutput.insert(outputFile);
            continue;
        }

        fs::path kleeFilePathTemplate;
        if (Paths::isSubPathOf(projectContext.projectPath, sourceFile)) {
            kleeFilePathTemplate = Paths::createNewDirForFile(sourceFile, projectContext.projectPath,
                                                              Paths::getUTBotFiles(projectContext));
        } else {
            LOG_S(WARNING)
            << "Source file " << sourceFile << " outside of project root " << projectContext.projectPath;
            kleeFilePathTemplate = Paths::createNewDirForFile(sourceFile, fs::path("/"),
                                                              Paths::getUTBotFiles(projectContext) /
                                                              "outside_of_project");
        }

        fs::path kleeFile = Paths::addSuffix(kleeFilePathTemplate, "_klee");
        objectInfo->kleeFilesInfo = std::make_shared<KleeFilesInfo>(kleeFile);

        if (CollectionUtils::containsKey(objectFileInfos, outputFile) && Paths::isObjectFile(outputFile)) {
            auto previusInfo = objectFileInfos[outputFile];
            if (previusInfo->command.getCommandLine() == objectInfo->command.getCommandLine()) {
                LOG_S(WARNING) << "Skip duplicate compile command for object file: " << outputFile;
            } else {
                LOG_S(WARNING) << "Skip second compile command for object file: " << outputFile;
            }
            continue;
        }

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
                        std::initializer_list<std::string>{targetObjectInfo->command.getBuildTool(),
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
        const fs::path &sourcePath = objectInfo->getSourcePath();

        sourceFileInfos[sourcePath].emplace_back(objectInfo);

    }
    for (auto &[sourceFile, objectInfos]: sourceFileInfos) {
        std::sort(objectInfos.begin(), objectInfos.end(), BuildDatabase::ObjectFileInfo::conflictPriorityMore);
    }
}

void ProjectBuildDatabase::initInfo(const nlohmann::json &linkCommandsJson, bool skipObjectWithoutSource) {
    for (nlohmann::json const &linkCommand: linkCommandsJson) {
        fs::path directory = linkCommand.at("directory").get<std::string>();
        std::vector<std::string> jsonArguments;
        if (linkCommand.contains("command")) {
            std::string command = linkCommand.at("command");
            jsonArguments = StringUtils::splitByWhitespaces(command);
        } else {
            jsonArguments = std::vector<std::string>(linkCommand.at("arguments"));
        }
        LOG_S(MAX) << "Processing link command: " << StringUtils::joinWith(jsonArguments, " ");
        if (StringUtils::endsWith(jsonArguments[0], "ranlib")) {
            LOG_S(MAX) << "Skip ranlib command";
            continue;
        }
        if (StringUtils::endsWith(jsonArguments[0], "cmake")) {
            LOG_S(MAX) << "Skip cmake command";
            continue;
        }
        std::transform(jsonArguments.begin(), jsonArguments.end(), jsonArguments.begin(),
                       [&directory](const std::string &argument) {
                           return tryConvertOptionToPath(argument, directory);
                       });

        mergeLibraryOptions(jsonArguments);

        utbot::LinkCommand command(jsonArguments, directory);
        fs::path const &output = command.getOutput();
        if (output.empty()) {
            LOG_S(WARNING) << "Empty output of command: " << command.toString();
        }

        auto targetInfo = targetInfos[output];
        if (targetInfo == nullptr) {
            targetInfo = targetInfos[output] = std::make_shared<TargetInfo>();
        } else {
            LOG_S(WARNING) << "Multiple commands for one file: " << output.string();
        }
        for (nlohmann::json const &jsonFile: linkCommand.at("files")) {
            auto filename = jsonFile.get<std::string>();
            fs::path currentFile = Paths::getFileFullPath(filename, command.getDirectory());
            if (ignoredOutput.count(currentFile)) {
                continue;
            }
            if (Paths::isObjectFile(currentFile)) {
                if (!CollectionUtils::containsKey(objectFileInfos, currentFile) &&
                    !CollectionUtils::containsKey(objectFileInfos,
                                                  relative(currentFile, directory))) {
                    std::string message =
                            "compile_commands.json doesn't contain a command for object file " +
                            currentFile.string();
                    if (skipObjectWithoutSource) {
                        LOG_S(WARNING) << message;
                        continue;
                    }
                    LOG_S(ERROR) << message;
                    throw CompilationDatabaseException(message);
                }
                if (CollectionUtils::containsKey(objectFileInfos, currentFile)) {
                    objectFileInfos[currentFile]->linkUnit = output;
                } else if (CollectionUtils::containsKey(objectFileInfos,
                                                        relative(currentFile, directory))) {
                    objectFileInfos[relative(currentFile, directory)]->linkUnit = output;
                }
            }
            targetInfo->addFile(currentFile);
        }
        targetInfo->commands.emplace_back(command);
    }
}


void ProjectBuildDatabase::filterInstalledFiles() {
    for (auto &it: targetInfos) {
        auto &linkFile = it.first;
        auto &targetInfo = it.second;
        CollectionUtils::OrderedFileSet fileset;
        targetInfo->installedFiles =
                CollectionUtils::filterOut(targetInfo->files, [this](fs::path const &file) {
                    return CollectionUtils::containsKey(targetInfos, file) ||
                           CollectionUtils::containsKey(objectFileInfos, file);
                });
        if (!targetInfo->installedFiles.empty()) {
            LOG_S(DEBUG) << "Target " << linkFile << " depends on " << targetInfo->installedFiles.size()
                         << " installed files";
        }
        CollectionUtils::erase_if(targetInfo->files, [&targetInfo](fs::path const &file) {
            return CollectionUtils::contains(targetInfo->installedFiles, file);
        });
    }
}

void ProjectBuildDatabase::addLocalSharedLibraries() {
    sharedLibrariesMap sharedLibraryFiles;
    for (const auto &[linkFile, linkUnit]: targetInfos) {
        if (Paths::isSharedLibraryFile(linkFile)) {
            auto withoutVersion = CompilationUtils::removeSharedLibraryVersion(linkFile);
            sharedLibraryFiles[withoutVersion.filename()][linkFile.parent_path()] = linkFile;
        }
    }
    for (auto &[linkFile, targetInfo]: targetInfos) {
        for (auto &command: targetInfo->commands) {
            addLibrariesForCommand(command, *targetInfo, sharedLibraryFiles);
        }
    }
    for (auto &[objectFile, objectInfo]: objectFileInfos) {
        addLibrariesForCommand(objectInfo->command, *objectInfo, sharedLibraryFiles, true);
    }
}

void ProjectBuildDatabase::fillTargetInfoParents() {
    CollectionUtils::MapFileTo<std::vector<fs::path>> parentTargets;
    for (const auto &[linkFile, linkUnit]: targetInfos) {
        for (const fs::path &dependencyFile: linkUnit->files) {
            if (Paths::isLibraryFile(dependencyFile)) {
                parentTargets[dependencyFile].emplace_back(linkFile);
            }
            if (Paths::isObjectFile(dependencyFile)) {
                objectFileTargets[dependencyFile].emplace_back(linkFile);
            }
        }
    }
    for (auto &[library, parents]: parentTargets) {
        if (!CollectionUtils::containsKey(targetInfos, library)) {
            std::string message =
                    "link_commands.json doesn't contain a command for building library: " +
                    library.string() + "\nReferenced from command for: " +
                    (parents.empty() ? "none" : parents[0].string());
            LOG_S(ERROR) << message;
            throw CompilationDatabaseException(message);
        }
        targetInfos[library]->parentLinkUnits = std::move(parents);
    }
}
