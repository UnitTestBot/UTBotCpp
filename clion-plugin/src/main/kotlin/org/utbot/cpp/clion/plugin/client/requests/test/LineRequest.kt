package org.utbot.cpp.clion.plugin.client.requests.test

import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import org.utbot.cpp.clion.plugin.UTBot
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub

class LineRequest(
    request: Testgen.LineRequest,
    project: Project,
) : BaseTestsRequest<Testgen.LineRequest>(request, project, UTBot.message("requests.line.description.progress")) {

    override val logMessage: String = "Sending request to generate tests for line."
    override fun getInfoMessage(): String = "Tests for line are generated!"

    override suspend fun TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Flow<Testgen.TestsResponse> =
        generateLineTests(request)
}
