package com.huawei.utbot.cpp.client.requests

import com.huawei.utbot.cpp.UTBot
import com.huawei.utbot.cpp.client.handlers.GenerateJsonHandler
import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt

class GenerateJsonFilesRequest(
    project: Project,
    request: Testgen.ProjectConfigRequest,
): BaseRequest<Testgen.ProjectConfigRequest, Flow<Testgen.ProjectConfigResponse>>(request, project) {
    override val logMessage: String = "Sending request to check project configuration."

    override suspend fun TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Flow<Testgen.ProjectConfigResponse> {
        return this.configureProject(request)
    }

    override suspend fun Flow<Testgen.ProjectConfigResponse>.handle(cancellationJob: Job?) {
        if (cancellationJob?.isActive == true) {
            GenerateJsonHandler(
                project,
                this,
                UTBot.message("requests.json.description.progress"),
                cancellationJob
            ).handle()
        }
    }
}
