package org.utbot.cpp.clion.plugin.client.requests.test

import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.grpc.GrpcRequestBuilder
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub

class AssertionRequest(
    params: GrpcRequestBuilder<Testgen.AssertionRequest>,
    project: Project
) : BaseTestsRequest<Testgen.AssertionRequest>(params, project, UTBot.message("requests.assertion.description.progress")) {
    override val id: String = "Generate for Assertion"

    override val logMessage: String = "Sending request to generate tests for Assertion"
    override fun getInfoMessage(): String = "Tests for assertion are generated!"

    override suspend fun TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Flow<Testgen.TestsResponse> =
        generateAssertionFailTests(request)
}
