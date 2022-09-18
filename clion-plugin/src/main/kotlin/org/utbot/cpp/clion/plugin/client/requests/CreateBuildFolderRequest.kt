package org.utbot.cpp.clion.plugin.client.requests

import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.client.ManagedClient
import org.utbot.cpp.clion.plugin.grpc.getProjectConfigGrpcRequest
import org.utbot.cpp.clion.plugin.client.handlers.CreateBuildDirHandler
import org.utbot.cpp.clion.plugin.grpc.Params
import org.utbot.cpp.clion.plugin.grpc.ParamsBuilder
import org.utbot.cpp.clion.plugin.utils.getCurrentClient
import org.utbot.cpp.clion.plugin.utils.activeProject
import org.utbot.cpp.clion.plugin.utils.client
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub

class CreateBuildDirRequest(
    params: Params<Testgen.ProjectConfigRequest>,
    project: Project,
    val client: ManagedClient,
) : BaseRequest<Testgen.ProjectConfigRequest, Flow<Testgen.ProjectConfigResponse>>(request, client.project) {
    override val logMessage: String = "Sending request to check project configuration."

    override suspend fun TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Flow<Testgen.ProjectConfigResponse> =
        this.configureProject(request)

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
