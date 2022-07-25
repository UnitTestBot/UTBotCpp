package org.utbot.cpp.clion.plugin.client.requests

import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.grpc.getProjectConfigGrpcRequest
import org.utbot.cpp.clion.plugin.client.handlers.CreateBuildDirHandler
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt

class CreateBuildDirRequest(
    project: Project,
    request: Testgen.ProjectConfigRequest,
): BaseRequest<Testgen.ProjectConfigRequest, Flow<Testgen.ProjectConfigResponse>>(request, project) {
    override val logMessage: String = "Sending request to check project configuration."

    constructor(e: AnActionEvent): this(e.project!!, getProjectConfigGrpcRequest(e.project!!, Testgen.ConfigMode.CREATE_BUILD_DIR))

    override suspend fun TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Flow<Testgen.ProjectConfigResponse> {
        return this.configureProject(request)
    }

    override suspend fun Flow<Testgen.ProjectConfigResponse>.handle(cancellationJob: Job?) {
        if (cancellationJob?.isActive == true) {
            CreateBuildDirHandler(
                project,
                this,
                UTBot.message("requests.buildDir.description.progress"),
                cancellationJob
            ).handle()
        }
    }
}
