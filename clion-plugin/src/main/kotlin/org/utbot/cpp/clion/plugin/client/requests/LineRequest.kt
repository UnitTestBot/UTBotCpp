package org.utbot.cpp.clion.plugin.client.requests

import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import org.utbot.cpp.clion.plugin.UTBot
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt

class LineRequest(
    request: Testgen.LineRequest,
    project: Project,
) : BaseTestsRequest<Testgen.LineRequest>(request, project, UTBot.message("requests.line.description.progress")) {
    override val logMessage: String = "Sending request to generate for LINE."
    override fun getInfoMessage(): String {
        return "Tests for line generated!"
    }

    override suspend fun TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Flow<Testgen.TestsResponse> =
        generateLineTests(request)
}
