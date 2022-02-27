package org.utbot.cpp.clion.plugin.client.requests

import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import org.utbot.cpp.clion.plugin.UTBot
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt

class ClassRequest(
    request: Testgen.ClassRequest,
    project: Project,
) : BaseTestsRequest<Testgen.ClassRequest>(request, project, UTBot.message("requests.class.description.progress")) {
    override val logMessage: String = "Sending request to generate tests for CLASS."
    override fun getInfoMessage(): String = "Class tests generated!"
    override suspend fun TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Flow<Testgen.TestsResponse> =
        generateClassTests(request)
}
