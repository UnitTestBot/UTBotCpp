/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "UserProjectConfiguration.h"

#include "Paths.h"
#include "environment/EnvironmentPaths.h"
#include "tasks/ShellExecTask.h"
#include "utils/Copyright.h"
#include "utils/FileSystemUtils.h"
#include "utils/ExecUtils.h"
#include "utils/LogUtils.h"
#include "utils/MakefileUtils.h"
#include "utils/StringUtils.h"

#include "loguru.h"

Status UserProjectConfiguration::CheckProjectConfiguration(const fs::path &buildDirPath,
                                                           ProjectConfigWriter const &writer) {
    if (!fs::exists(buildDirPath)) {
        writer.writeResponse(ProjectConfigStatus::BUILD_DIR_NOT_FOUND);
    } else if (!fs::exists(getCompileCommandsJsonPath(buildDirPath))) {
        writer.writeResponse(ProjectConfigStatus::COMPILE_COMMANDS_JSON_NOT_FOUND);
    } else if (!fs::exists(getLinkCommandsJsonPath(buildDirPath))) {
        writer.writeResponse(ProjectConfigStatus::LINK_COMMANDS_JSON_NOT_FOUND);
    } else {
        writer.writeResponse(ProjectConfigStatus::IS_OK);
    }
    return Status::OK;
}

Status UserProjectConfiguration::RunBuildDirectoryCreation(const fs::path &buildDirPath,
                                                           ProjectConfigWriter const &writer) {
    try {
        fs::create_directories(buildDirPath);
        createBearShScript(buildDirPath);
        writer.writeResponse(ProjectConfigStatus::IS_OK);
    } catch (fs::filesystem_error const &e) {
        writer.writeResponse(ProjectConfigStatus::BUILD_DIR_CREATION_FAILED, e.code().message());
    }
    return Status::OK;
}


Status
UserProjectConfiguration::RunProjectConfigurationCommands(const fs::path &buildDirPath,
                                                          const string &projectName,
                                                          const string &cmakeOptions,
                                                          ProjectConfigWriter const &writer) {
    try {
        fs::path bearShPath = createBearShScript(buildDirPath);

        ShellExecTask::ExecutionParameters cmakeParams(Paths::getCMake(), {cmakeOptions, ".."});
        ShellExecTask::ExecutionParameters bearMakeParams(Paths::getBear(), {Paths::getMake(), MakefileUtils::threadFlag()});


        fs::path cmakeListsPath = getCmakeListsPath(buildDirPath);
        if (fs::exists(cmakeListsPath)) {
            LOG_S(INFO) << "Configure cmake project";
            RunProjectConfigurationCommand(buildDirPath, cmakeParams, projectName, writer);
        } else {
            LOG_S(INFO) << "CMakeLists.txt not found in root project directory: " << cmakeListsPath
                        << ". Skipping cmake step.";
        }
        LOG_S(INFO) << "Configure make project";
        RunProjectConfigurationCommand(buildDirPath, bearMakeParams, projectName, writer);
        writer.writeResponse(ProjectConfigStatus::IS_OK);
    } catch (const std::exception &e) {
        fs::remove(getCompileCommandsJsonPath(buildDirPath));
        fs::remove(getLinkCommandsJsonPath(buildDirPath));
        writer.writeResponse(ProjectConfigStatus::RUN_JSON_GENERATION_FAILED, e.what());
    }
    return Status::OK;
}

void UserProjectConfiguration::RunProjectConfigurationCommand(const fs::path &buildDirPath,
                                                              const ShellExecTask::ExecutionParameters &params,
                                                              const string &projectName,
                                                              const ProjectConfigWriter &writer) {
    auto [out, status, _] = ShellExecTask::runShellCommandTask(params, buildDirPath, projectName, true, true);
    if (status != 0) {
        auto logFilePath = LogUtils::writeLog(out, Paths::getTmpDir(projectName), "project-import");
        string message = StringUtils::stringFormat(
            "Running command \"%s\" failed. See more info in logs.", params.toString());
        throw std::runtime_error(message);
    }
}

fs::path UserProjectConfiguration::getCmakeListsPath(const fs::path &buildDirPath) {
    return buildDirPath.parent_path() / "CMakeLists.txt";
}

fs::path UserProjectConfiguration::getCompileCommandsJsonPath(const fs::path &buildDirPath) {
    return buildDirPath / "compile_commands.json";
}

fs::path UserProjectConfiguration::getLinkCommandsJsonPath(const fs::path &buildDirPath) {
    return buildDirPath / "link_commands.json";
}

fs::path UserProjectConfiguration::getBearShScriptPath(const fs::path &buildDirPath) {
    return buildDirPath / "bear.sh";
}

static string getBearShEnvironmentSetting() {
    auto libraryDirs = {
            Paths::getUTBotDebsInstallDir() / "usr/lib/x86_64-linux-gnu",
            Paths::getUTBotDebsInstallDir() / "lib/x86_64-linux-gnu",
            Paths::getUTBotInstallDir() / "lib"
    };
    return "export LD_LIBRARY_PATH=" + StringUtils::joinWith(libraryDirs, ":");
}

fs::path UserProjectConfiguration::createBearShScript(const fs::path &buildDirPath) {
    fs::path bearShPath = getBearShScriptPath(buildDirPath);
    std::string script =
        StringUtils::stringFormat("#!/bin/bash\n"
                                  "%s\n"
                                  "%s\n"
                                  "%s %s \"$@\"",
                                  Copyright::GENERATED_SH_HEADER, getBearShEnvironmentSetting(),
                                  Paths::getPython(), Paths::getBear());
    FileSystemUtils::writeToFile(bearShPath, script);
    fs::permissions(bearShPath, fs::perms::all);
    return bearShPath;
}
