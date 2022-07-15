package com.huawei.utbot.cpp.client.requests

import com.huawei.utbot.cpp.UTBot
import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt

class FunctionRequest(
    request: Testgen.FunctionRequest,
    project: Project,
) : BaseTestsRequest<Testgen.FunctionRequest>(request, project, UTBot.message("requests.function.description.progress")) {

    override val logMessage: String = "Sending request to generate tests for FUNCTION."
    override fun getInfoMessage(): String {
        return "Function tests generated!"
    }
    override suspend fun TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Flow<Testgen.TestsResponse> =
        generateFunctionTests(request)
}
