package com.huawei.utbot.cpp.client.Requests

import com.huawei.utbot.cpp.UTBot
import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt

class ClassRequest(
    request: Testgen.ClassRequest,
    project: Project,
) : BaseTestsRequest<Testgen.ClassRequest>(request, project, UTBot.message("requests.class.description.progress")) {
    override val logMessage: String = "Sending request to generate tests for CLASS."
    override suspend fun TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Flow<Testgen.TestsResponse> =
        generateClassTests(request)
}
