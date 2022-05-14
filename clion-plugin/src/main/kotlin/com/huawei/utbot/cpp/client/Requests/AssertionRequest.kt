package com.huawei.utbot.cpp.client.Requests

import com.huawei.utbot.cpp.UTBot
import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt

class AssertionRequest(
    request: Testgen.AssertionRequest,
    project: Project,
) : BaseTestsRequest<Testgen.AssertionRequest>(request, project, UTBot.message("requests.assertion.description.progress")) {
    override val logMessage: String = "Sending request to generate tests for ASSERTION."
    override suspend fun TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Flow<Testgen.TestsResponse> =
        generateAssertionFailTests(request)
}
