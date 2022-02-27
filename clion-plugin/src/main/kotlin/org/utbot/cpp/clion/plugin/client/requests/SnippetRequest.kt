package org.utbot.cpp.clion.plugin.client.requests

import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import org.utbot.cpp.clion.plugin.UTBot
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt

class SnippetRequest(
    request: Testgen.SnippetRequest,
    project: Project
) : BaseTestsRequest<Testgen.SnippetRequest>(request, project, UTBot.message("requests.snippet.description.progress")) {
    override val logMessage: String = "Sending request to generate for SNIPPET."
    override fun getInfoMessage(): String {
        return "Snippet tests generated!"
    }
    override suspend fun TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Flow<Testgen.TestsResponse> =
        generateSnippetTests(request)
}
