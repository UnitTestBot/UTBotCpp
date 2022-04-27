package com.huawei.utbot.cpp.client.Requests

import kotlinx.coroutines.Job
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt

class FunctionReturnTypeRequest(
    request: Testgen.FunctionRequest,
    val processReturnType: suspend (Testgen.FunctionTypeResponse)->(Unit)
) : BaseRequest<Testgen.FunctionRequest, Testgen.FunctionTypeResponse>(request) {
    override val logMessage: String = "Sending request to generate tests for CLASS."

    override suspend fun Testgen.FunctionTypeResponse.handle(cancellationJob: Job?) {
        processReturnType(this)
    }

    override suspend fun TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Testgen.FunctionTypeResponse {
        return getFunctionReturnType(request)
    }
}
