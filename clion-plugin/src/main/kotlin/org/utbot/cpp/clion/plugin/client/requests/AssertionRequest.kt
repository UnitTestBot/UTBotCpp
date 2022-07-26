package org.utbot.cpp.clion.plugin.client.requests

import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import org.utbot.cpp.clion.plugin.UTBot
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt

class AssertionRequest(
    request: Testgen.AssertionRequest,
    project: Project,
) : BaseTestsRequest<Testgen.AssertionRequest>(request, project, UTBot.message("requests.assertion.description.progress")) {
    override val logMessage: String = "Sending request to generate tests for ASSERTION."
    override fun getInfoMessage(): String = "Assertion tests generated!"
    override suspend fun TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Flow<Testgen.TestsResponse> =
        generateAssertionFailTests(request)
}
