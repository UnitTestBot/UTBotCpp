/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_USERPROJECTCONFIGURATION_H
#define UNITTESTBOT_USERPROJECTCONFIGURATION_H

#include "streams/ProjectConfigWriter.h"
#include "tasks/ShellExecTask.h"

#include <grpcpp/grpcpp.h>
#include <protobuf/testgen.grpc.pb.h>

#include "utils/path/FileSystemPath.h"


using grpc::Status;
using std::string;
using testsgen::ProjectConfigStatus;

class UserProjectConfiguration {
public:
    static Status CheckProjectConfiguration(const fs::path &buildDirPath,
                                            ProjectConfigWriter const &writer);

    static Status RunBuildDirectoryCreation(const fs::path &buildDirPath,
                                            ProjectConfigWriter const &writer);

    static Status RunProjectConfigurationCommands(const fs::path &buildDirPath,
                                                  const string &projectName,
                                                  const string &cmakeOptions,
                                                  ProjectConfigWriter const &writer);

private:
    static void RunProjectConfigurationCommand(const fs::path &buildDirPath,
                                               const ShellExecTask::ExecutionParameters &params,
                                               const string &projectName,
                                               const ProjectConfigWriter &writer);

    static fs::path getCmakeListsPath(const fs::path &buildDirPath);

    static fs::path getCompileCommandsJsonPath(const fs::path &buildDirPath);

    static fs::path getLinkCommandsJsonPath(const fs::path &buildDirPath);

    static fs::path getBearShScriptPath(const fs::path &buildDirPath);

    static fs::path createBearShScript(const fs::path &buildDirPath);
};


#endif // UNITTESTBOT_USERPROJECTCONFIGURATION_H
