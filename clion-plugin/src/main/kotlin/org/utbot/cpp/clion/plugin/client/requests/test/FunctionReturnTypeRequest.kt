package org.utbot.cpp.clion.plugin.client.requests.test

import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import org.utbot.cpp.clion.plugin.client.requests.BaseRequest
import testsgen.Testgen
import testsgen.Testgen.FunctionTypeResponse
import testsgen.TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub

class FunctionReturnTypeRequest(
    request: Testgen.FunctionRequest,
    project: Project,
    val processReturnType: suspend (FunctionTypeResponse)->(Unit)
) : BaseRequest<Testgen.FunctionRequest, FunctionTypeResponse>(request, project) {

    override val logMessage: String = "Sending request to get function return type"

    override suspend fun FunctionTypeResponse.handle(cancellationJob: Job?) = processReturnType(this)

    override suspend fun TestsGenServiceCoroutineStub.send(cancellationJob: Job?): FunctionTypeResponse =
        getFunctionReturnType(request)
}
