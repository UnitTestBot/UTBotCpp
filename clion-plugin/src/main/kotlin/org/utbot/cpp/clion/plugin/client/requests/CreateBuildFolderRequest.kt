package org.utbot.cpp.clion.plugin.client.requests

import com.intellij.openapi.actionSystem.AnActionEvent
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.client.Client
import org.utbot.cpp.clion.plugin.grpc.getProjectConfigGrpcRequest
import org.utbot.cpp.clion.plugin.client.handlers.CreateBuildDirHandler
import org.utbot.cpp.clion.plugin.utils.getCurrentClient
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt

class CreateBuildDirRequest(
    val client: Client,
    request: Testgen.ProjectConfigRequest,
) : BaseRequest<Testgen.ProjectConfigRequest, Flow<Testgen.ProjectConfigResponse>>(request, client.project) {
    override val logMessage: String = "Sending request to check project configuration."

    constructor(e: AnActionEvent) : this(
        e.project?.getCurrentClient() ?: error("project is null for event: $e"),
        getProjectConfigGrpcRequest(e.project!!, Testgen.ConfigMode.CREATE_BUILD_DIR)
    )

    override suspend fun TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Flow<Testgen.ProjectConfigResponse> {
        return this.configureProject(request)
    }

    override suspend fun Flow<Testgen.ProjectConfigResponse>.handle(cancellationJob: Job?) {
        if (cancellationJob?.isActive == true) {
            CreateBuildDirHandler(
                client,
                this,
                UTBot.message("requests.buildDir.description.progress"),
                cancellationJob
            ).handle()
        }
    }
}
