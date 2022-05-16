package com.huawei.utbot.cpp.client.requests

import com.huawei.utbot.cpp.UTBot
import com.huawei.utbot.cpp.client.handlers.CheckProjectConfigurationHandler
import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt

class CheckProjectConfigurationRequest(
    val project: Project,
    request: Testgen.ProjectConfigRequest,
): BaseRequest<Testgen.ProjectConfigRequest, Flow<Testgen.ProjectConfigResponse>>(request) {
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
