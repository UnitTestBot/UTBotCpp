package org.utbot.cpp.clion.plugin.client.requests.test

import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.grpc.GrpcRequestBuilder
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub

class ClassRequest(
    params: GrpcRequestBuilder<Testgen.ClassRequest>,
    project: Project,
) : BaseTestsRequest<Testgen.ClassRequest>(params, project, UTBot.message("requests.class.description.progress")) {
    override val id: String = "Generate for Class"

    override val logMessage: String = "Sending request to generate tests for class"
    override fun getInfoMessage(): String = "Tests for class are generated!"

    override suspend fun TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Flow<Testgen.TestsResponse> =
        generateClassTests(request)
}
