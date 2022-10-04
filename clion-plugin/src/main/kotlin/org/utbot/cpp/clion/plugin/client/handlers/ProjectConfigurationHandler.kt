package org.utbot.cpp.clion.plugin.client.handlers

import com.intellij.openapi.project.Project
import kotlin.io.path.name
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.actions.AskServerToGenerateBuildDir
import org.utbot.cpp.clion.plugin.actions.AskServerToGenerateJsonForProjectConfiguration
import org.utbot.cpp.clion.plugin.client.ManagedClient
import org.utbot.cpp.clion.plugin.client.requests.CheckProjectConfigurationRequest
import org.utbot.cpp.clion.plugin.grpc.ParamsBuilder
import org.utbot.cpp.clion.plugin.settings.settings
import org.utbot.cpp.clion.plugin.utils.logger
import org.utbot.cpp.clion.plugin.utils.notifyError
import org.utbot.cpp.clion.plugin.utils.notifyInfo
import org.utbot.cpp.clion.plugin.utils.notifyUnknownResponse
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
                notifyInfo(UTBot.message("notify.title.configured"), UTBot.message("notify.configured"), project)
            }
            Testgen.ProjectConfigStatus.BUILD_DIR_NOT_FOUND -> {
                notifyError(
                    UTBot.message("notify.title.missingBuildDir"),
                    UTBot.message("notify.missingBuildDir"),
                    project,
                    AskServerToGenerateBuildDir()
                )
            }
            Testgen.ProjectConfigStatus.LINK_COMMANDS_JSON_NOT_FOUND, Testgen.ProjectConfigStatus.COMPILE_COMMANDS_JSON_NOT_FOUND -> {
                val missingFileName =
                    if (response.type == Testgen.ProjectConfigStatus.LINK_COMMANDS_JSON_NOT_FOUND) "link_commands.json" else "compile_commands.json"
                notifyError(
                    UTBot.message("notify.title.notConfigured"),
                    UTBot.message("notify.missing.cdb.files", missingFileName),
                    project,
                    AskServerToGenerateJsonForProjectConfiguration(),
                )
            }
            Testgen.ProjectConfigStatus.BUILD_DIR_SAME_AS_PROJECT -> {
                val message = response.message
                logger.warn(message)
                notifyError(UTBot.message("notify.title.error"), "$message ${UTBot.message("uri.wiki")}", project)
            }
            else -> notifyUnknownResponse(response, project)
        }
    }
}

class CreateBuildDirHandler(
    val client: ManagedClient,
    grpcStream: Flow<Testgen.ProjectConfigResponse>,
    progressName: String,
    cancellationJob: Job
) : ProjectConfigResponseHandler(client.project, grpcStream, progressName, cancellationJob) {
    override fun handle(response: Testgen.ProjectConfigResponse) {
        when (response.type) {
            Testgen.ProjectConfigStatus.IS_OK -> {
                notifyInfo(
                    UTBot.message("notify.build.dir.created.title"),
                    UTBot.message("notify.build.dir.created", project.settings.buildDirPath.name),
                    project
                )

                CheckProjectConfigurationRequest(
                    ParamsBuilder(project).buildProjectConfigRequestParams(Testgen.ConfigMode.CHECK),
                    project
                ).also {
                    client.executeRequest(it)
                }
            }
            Testgen.ProjectConfigStatus.BUILD_DIR_CREATION_FAILED -> {
                notifyError(
                    UTBot.message("notify.failed.to.create.build.dir.title"),
                    "",
                    project
                )
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
            Testgen.ProjectConfigStatus.IS_OK -> notifyInfo(
                UTBot.message("notify.title.configured"),
                UTBot.message("notify.configured"),
                project
            )
            Testgen.ProjectConfigStatus.RUN_JSON_GENERATION_FAILED -> notifyError(
                UTBot.message("notify.title.notConfigured"),
                response.message
            )
            else -> notifyUnknownResponse(response, project)
        }
        markDirtyAndRefresh(project.settings.buildDirPath)
    }
}
