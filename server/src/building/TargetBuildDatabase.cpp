#include "TargetBuildDatabase.h"

#include "loguru.h"
#include "utils/GrpcUtils.h"
#include "utils/GenerationUtils.h"

TargetBuildDatabase::TargetBuildDatabase(BuildDatabase *baseBuildDatabase, const std::string &targetOrSourcePath) :
        BuildDatabase(baseBuildDatabase) {
    if (Paths::isSourceFile(targetOrSourcePath)) {
        target = baseBuildDatabase->getRootForSource(targetOrSourcePath);
    } else if (targetOrSourcePath == GrpcUtils::UTBOT_AUTO_TARGET_PATH || targetOrSourcePath.empty()) {
        target = baseBuildDatabase->getRootForFirstSource();
    } else {
        auto new_target = GenerationUtils::findTarget(baseBuildDatabase->getAllTargets(), targetOrSourcePath);
        if (new_target.has_value()) {
            target = new_target.value();
        } else {
            throw CompilationDatabaseException("Can't find target: " + targetOrSourcePath);
        }
    }

    LOG_S(INFO) << StringUtils::stringFormat("Chosen target: %s", target);

    {
        auto objectFilesList = baseBuildDatabase->getArchiveObjectFiles(target);
        for (const auto &objectFilePath: objectFilesList) {
            auto objectFileInfo = baseBuildDatabase->getClientCompilationObjectInfo(objectFilePath);
            sourceFileInfos[objectFileInfo->getSourcePath()].push_back(objectFileInfo);
            LOG_IF_S(DEBUG, sourceFileInfos[objectFileInfo->getSourcePath()].size() > 1)
            << "Multiple compile commands for file \"" << objectFileInfo->getSourcePath() << "\" in target \""
            << target.string() << "\"";
            objectFileInfos[objectFileInfo->getOutputFile()] = objectFileInfo;
            objectFileTargets[objectFileInfo->getOutputFile()] = baseBuildDatabase->getTargetPathsForObjectFile(
                    objectFileInfo->getOutputFile());
        }
    }

    {
        auto targetFilesList = baseBuildDatabase->getArchiveTargetFiles(target);
        for (const auto &objectFilePath: targetFilesList) {
            targetInfos[objectFilePath] = baseBuildDatabase->getTargetInfo(objectFilePath);
        }
    }

    createClangCompileCommandsJson();
}

std::vector<std::shared_ptr<BuildDatabase::TargetInfo>> TargetBuildDatabase::getRootTargets() const {
    if (!hasAutoTarget()) {
        return {targetInfos.at(target)};
    }
    return BuildDatabase::getRootTargets();
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

bool TargetBuildDatabase::hasAutoTarget() const {
    return isAutoTarget;
}

fs::path TargetBuildDatabase::getTargetPath() const {
    return target;
}
