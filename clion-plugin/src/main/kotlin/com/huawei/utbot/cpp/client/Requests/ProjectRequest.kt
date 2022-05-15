package com.huawei.utbot.cpp.client.Requests

import com.huawei.utbot.cpp.UTBot
import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt

class ProjectRequest(
    request: Testgen.ProjectRequest,
    project: Project,
) : BaseTestsRequest<Testgen.ProjectRequest>(request, project, UTBot.message("requests.project.description.progress")) {
    override val target: String = "Project"
    override val logMessage: String = "Sending request to generate for PROJECT."
    override suspend fun TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Flow<Testgen.TestsResponse> =
        generateProjectTests(request)
}
