package com.huawei.utbot.cpp.client.Requests

import com.huawei.utbot.cpp.UTBot
import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt

class SnippetRequest(
    request: Testgen.SnippetRequest,
    project: Project
) : BaseTestsRequest<Testgen.SnippetRequest>(request, project, UTBot.message("requests.snippet.description.progress")) {
    override val target: String = "Snippet"
    override val logMessage: String = "Sending request to generate for SNIPPET."
    override suspend fun TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Flow<Testgen.TestsResponse> =
        generateSnippetTests(request)
}
