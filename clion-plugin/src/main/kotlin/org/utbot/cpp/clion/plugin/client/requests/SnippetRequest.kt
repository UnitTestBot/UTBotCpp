package org.utbot.cpp.clion.plugin.client.requests

import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import org.utbot.cpp.clion.plugin.UTBot
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub

class SnippetRequest(
    request: Testgen.SnippetRequest,
    project: Project
) : BaseTestsRequest<Testgen.SnippetRequest>(request, project, UTBot.message("requests.snippet.description.progress")) {

    override val logMessage: String = "Sending request to generate tests for snippet"
    override fun getInfoMessage(): String = "Tests for snippet are generated!"

    override suspend fun TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Flow<Testgen.TestsResponse> =
        generateSnippetTests(request)
}
