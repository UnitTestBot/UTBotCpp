package org.utbot.cpp.clion.plugin.client.requests

import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt

class FunctionReturnTypeRequest(
    project: Project,
    request: Testgen.FunctionRequest,
    val processReturnType: suspend (Testgen.FunctionTypeResponse)->(Unit)
) : BaseRequest<Testgen.FunctionRequest, Testgen.FunctionTypeResponse>(request, project) {
    override val logMessage: String = "Sending request to generate tests for CLASS."

    override suspend fun Testgen.FunctionTypeResponse.handle(cancellationJob: Job?) {
        processReturnType(this)
    }

    override suspend fun TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Testgen.FunctionTypeResponse {
        return getFunctionReturnType(request)
    }
}
