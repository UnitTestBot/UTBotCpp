#include "TargetBuildDatabase.h"


TargetBuildDatabase::TargetBuildDatabase(BuildDatabase &baseBuildDatabase,
                                         const std::string &_target) :
        serverBuildDir(baseBuildDatabase.serverBuildDir),
        projectContext(baseBuildDatabase.projectContext),
        buildCommandsJsonPath(baseBuildDatabase.buildCommandsJsonPath),
        linkCommandsJsonPath(baseBuildDatabase.linkCommandsJsonPath),
        compileCommandsJsonPath(baseBuildDatabase.compileCommandsJsonPath),
        target(_target),
        isAutoTarget(_target == GrpcUtils::UTBOT_AUTO_TARGET_PATH) {
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

    {
        auto targetFilesList = baseBuildDatabase.getArchiveTargetFiles(target);
        for (const auto &objectFilePath: targetFilesList) {
            targetInfos[objectFilePath] = baseBuildDatabase.targetInfos[objectFilePath];
        }
    }

    compileCommands_temp = baseBuildDatabase.compileCommands_temp;
    createClangCompileCommandsJson();
}

std::shared_ptr<BuildDatabase> BuildDatabase::createForSourceOrTarget(const std::string &_targetOrSourcePath) {
    fs::path _target;
    if (Paths::isSourceFile(_targetOrSourcePath)) {
        _target = getRootForSource(_targetOrSourcePath);
    } else if (_targetOrSourcePath == GrpcUtils::UTBOT_AUTO_TARGET_PATH || _targetOrSourcePath.empty()) {
        _target = getRootForFirstSource();
    } else {
        auto new_target = GenerationUtils::findTarget(getAllTargets(), _targetOrSourcePath);
        if (new_target.has_value()) {
            _target = new_target.value();
        } else {
            throw CompilationDatabaseException("Can't find target: " + _targetOrSourcePath);
        }
    }
    return std::make_shared<BuildDatabase>(std::move(BuildDatabase(*this, _target)));
}

std::vector<fs::path> TargetBuildDatabase::getTargetPathsForSourceFile(const fs::path &sourceFilePath) const {
    if (!hasAutoTarget()) {
        return {target};
    }
    return BuildDatabase::getTargetPathsForSourceFile(sourceFilePath);
}


std::vector<fs::path> TargetBuildDatabase::getTargetPathsForObjectFile(const fs::path &objectFile) const {
    if (!hasAutoTarget()) {
        return {target};
    }
    return BuildDatabase::getTargetPathsForObjectFile(objectFile);
}
