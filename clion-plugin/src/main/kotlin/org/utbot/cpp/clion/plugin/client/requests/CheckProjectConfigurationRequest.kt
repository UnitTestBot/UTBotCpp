package org.utbot.cpp.clion.plugin.client.requests

import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.client.handlers.CheckProjectConfigurationHandler
import org.utbot.cpp.clion.plugin.grpc.Params
import org.utbot.cpp.clion.plugin.grpc.ParamsBuilder
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt

class CheckProjectConfigurationRequest(
    params: Params<Testgen.ProjectConfigRequest>,
    project: Project,
) : BaseRequest<Testgen.ProjectConfigRequest, Flow<Testgen.ProjectConfigResponse>>(params, project) {
    override val id: String = "Configure Project"

    override val logMessage: String = "Sending request to check project configuration."

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
