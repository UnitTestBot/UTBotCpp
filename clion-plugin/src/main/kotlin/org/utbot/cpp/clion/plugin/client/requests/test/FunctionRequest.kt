package org.utbot.cpp.clion.plugin.client.requests.test

import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.grpc.GrpcRequestBuilder
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub

class FunctionRequest(
    params: GrpcRequestBuilder<Testgen.FunctionRequest>,
    project: Project,
) : BaseTestsRequest<Testgen.FunctionRequest>(params, project, UTBot.message("requests.function.description.progress")) {
    override val id: String = "Generate for Function"

    override val logMessage: String = "Sending request to generate tests for function."
    override fun getInfoMessage(): String = "Tests for function are generated!"

    override suspend fun TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Flow<Testgen.TestsResponse> =
        generateFunctionTests(request)
}
