package org.utbot.cpp.clion.plugin.client.handlers

import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.actions.AskServerToGenerateBuildDir
import org.utbot.cpp.clion.plugin.actions.AskServerToGenerateJsonForProjectConfiguration
import org.utbot.cpp.clion.plugin.client.Client
import org.utbot.cpp.clion.plugin.client.requests.CheckProjectConfigurationRequest
import org.utbot.cpp.clion.plugin.grpc.getProjectConfigGrpcRequest
import org.utbot.cpp.clion.plugin.settings.settings
import org.utbot.cpp.clion.plugin.utils.logger
import org.utbot.cpp.clion.plugin.utils.notifyError
import org.utbot.cpp.clion.plugin.utils.notifyInfo
import org.utbot.cpp.clion.plugin.utils.notifyUnknownResponse
import org.utbot.cpp.clion.plugin.utils.notifyWarning
import org.utbot.cpp.clion.plugin.utils.markDirtyAndRefresh
import testsgen.Testgen

abstract class ProjectConfigResponseHandler(
    project: Project,
    grpcStream: Flow<Testgen.ProjectConfigResponse>,
    progressName: String,
    cancellationJob: Job
) : StreamHandlerWithProgress<Testgen.ProjectConfigResponse>(project, grpcStream, progressName, cancellationJob) {
    override fun Testgen.ProjectConfigResponse.getProgress() = progress
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
                    "Project is not configured properly: file $missingFileName is missed in the build directory",
                    project,
                    AskServerToGenerateJsonForProjectConfiguration(),
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
    val client: Client,
    grpcStream: Flow<Testgen.ProjectConfigResponse>,
    progressName: String,
    cancellationJob: Job
) : ProjectConfigResponseHandler(client.project, grpcStream, progressName, cancellationJob) {
    override fun handle(response: Testgen.ProjectConfigResponse) {
        when (response.type) {
            Testgen.ProjectConfigStatus.IS_OK -> {
                notifyInfo("Build directory was created!", project)

                CheckProjectConfigurationRequest(
                    getProjectConfigGrpcRequest(project, Testgen.ConfigMode.CHECK),
                    project,
                ).also {
                    if (!client.isDisposed) {
                        client.executeRequestIfNotDisposed(it)
                    }
                }
            }
            Testgen.ProjectConfigStatus.BUILD_DIR_CREATION_FAILED -> {
                notifyInfo("Failed to create build directory! ${response.message}", project)
            }
            else -> notifyUnknownResponse(response, project)
        }
        markDirtyAndRefresh(project.settings.buildDirPath)
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
        markDirtyAndRefresh(project.settings.buildDirPath)
    }
}
