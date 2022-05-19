package com.huawei.utbot.cpp.client.handlers

import com.huawei.utbot.cpp.UTBot
import com.huawei.utbot.cpp.actions.AskServerToGenerateBuildDir
import com.huawei.utbot.cpp.actions.AskServerToGenerateJsonForProjectConfiguration
import com.huawei.utbot.cpp.utils.getClient
import com.huawei.utbot.cpp.utils.logger
import com.huawei.utbot.cpp.utils.notifyError
import com.huawei.utbot.cpp.utils.notifyInfo
import com.huawei.utbot.cpp.utils.notifyUnknownResponse
import com.huawei.utbot.cpp.utils.notifyWarning
import com.huawei.utbot.cpp.utils.refreshAndFindIOFile
import com.huawei.utbot.cpp.utils.utbotSettings
import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import testsgen.Testgen
import testsgen.Util

abstract class ProjectConfigResponseHandler(
    project: Project,
    grpcStream: Flow<Testgen.ProjectConfigResponse>,
    progressName: String,
    cancellationJob: Job
) : StreamHandlerWithProgress<Testgen.ProjectConfigResponse>(project, grpcStream, progressName, cancellationJob) {
    override fun onLastResponse(response: Testgen.ProjectConfigResponse?) {
        if (response == null) {
            project.logger.error { "No responses from server!" }
            return
        }
        handle(response)
    }

    abstract fun handle(response: Testgen.ProjectConfigResponse)
}

class CheckProjectConfigurationHandler(
    project: Project,
    grpcStream: Flow<Testgen.ProjectConfigResponse>,
    progressName: String,
    cancellationJob: Job
) : ProjectConfigResponseHandler(project, grpcStream, progressName, cancellationJob) {
    override fun Testgen.ProjectConfigResponse.getProgress(): Util.Progress {
        return progress
    }

    override fun handle(response: Testgen.ProjectConfigResponse) {
        when (response.type) {
            Testgen.ProjectConfigStatus.IS_OK -> {
                notifyInfo("Project is configured!", project)
            }
            Testgen.ProjectConfigStatus.BUILD_DIR_NOT_FOUND -> {
                notifyError(
                    "Project build dir not found! ${response.message}",
                    project,
                    AskServerToGenerateBuildDir()
                )
            }
            Testgen.ProjectConfigStatus.LINK_COMMANDS_JSON_NOT_FOUND, Testgen.ProjectConfigStatus.COMPILE_COMMANDS_JSON_NOT_FOUND -> {
                val missingFileName =
                    if (response.type == Testgen.ProjectConfigStatus.LINK_COMMANDS_JSON_NOT_FOUND) "link_commands.json" else "compile_commands.json"
                notifyError(
                    "Project is not configured properly: $missingFileName is missing in the build folder.",
                    project, AskServerToGenerateJsonForProjectConfiguration()
                )
            }
            Testgen.ProjectConfigStatus.BUILD_DIR_SAME_AS_PROJECT -> {
                val message = response.message
                logger.warn(message)
                notifyWarning("$message ${UTBot.message("uri.wiki")}", project)
            }
            else -> notifyUnknownResponse(response, project)
        }
    }
}

class CreateBuildDirHandler(
    project: Project,
    grpcStream: Flow<Testgen.ProjectConfigResponse>,
    progressName: String,
    cancellationJob: Job
) : ProjectConfigResponseHandler(project, grpcStream, progressName, cancellationJob) {
    override fun Testgen.ProjectConfigResponse.getProgress(): Util.Progress {
        return progress
    }

    override fun handle(response: Testgen.ProjectConfigResponse) {
        when (response.type) {
            Testgen.ProjectConfigStatus.IS_OK -> {
                notifyInfo("Build dir was created!", project)
                project.getClient().configureProject()
            }
            Testgen.ProjectConfigStatus.BUILD_DIR_CREATION_FAILED -> {
                notifyInfo("Failed to create build dir! ${response.message}", project)
            }
            else -> notifyUnknownResponse(response, project)
        }
        refreshAndFindIOFile(project.utbotSettings.buildDirPath)
    }
}

class GenerateJsonHandler(
    project: Project,
    grpcStream: Flow<Testgen.ProjectConfigResponse>,
    progressName: String,
    cancellationJob: Job
) : ProjectConfigResponseHandler(project, grpcStream, progressName, cancellationJob) {
    override fun handle(response: Testgen.ProjectConfigResponse) {
        when (response.type) {
            Testgen.ProjectConfigStatus.IS_OK -> notifyInfo("Successfully configured project!", project)
            Testgen.ProjectConfigStatus.RUN_JSON_GENERATION_FAILED -> notifyError(
                "UTBot tried to configure project, but failed with the " +
                        "following message: ${response.message}", project
            )
            else -> notifyUnknownResponse(response, project)
        }
        refreshAndFindIOFile(project.utbotSettings.buildDirPath)
    }

    override fun Testgen.ProjectConfigResponse.getProgress() = progress
}
