package org.utbot.cpp.clion.plugin.client.requests

import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.grpc.getProjectConfigGrpcRequest
import org.utbot.cpp.clion.plugin.client.handlers.CheckProjectConfigurationHandler
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt

class CheckProjectConfigurationRequest(
    request: Testgen.ProjectConfigRequest,
    project: Project,
): BaseRequest<Testgen.ProjectConfigRequest, Flow<Testgen.ProjectConfigResponse>>(request, project) {
    override val logMessage: String = "Sending request to check project configuration."

    constructor(project: Project): this(getProjectConfigGrpcRequest(project, Testgen.ConfigMode.CHECK), project)

    override suspend fun TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Flow<Testgen.ProjectConfigResponse> {
        return this.configureProject(request)
    }

    override suspend fun Flow<Testgen.ProjectConfigResponse>.handle(cancellationJob: Job?) {
        if (cancellationJob?.isActive == true) {
            CheckProjectConfigurationHandler(
                project,
                this,
                UTBot.message("requests.check.description.progress"),
                cancellationJob
            ).handle()
        }
    }
}
