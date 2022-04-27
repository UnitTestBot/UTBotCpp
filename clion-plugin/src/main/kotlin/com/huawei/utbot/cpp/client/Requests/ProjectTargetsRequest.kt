package com.huawei.utbot.cpp.client.Requests

import kotlinx.coroutines.Job
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt

class ProjectTargetsRequest(
    request: Testgen.ProjectTargetsRequest,
    val processTargets: suspend (Testgen.ProjectTargetsResponse)->Unit
): BaseRequest<Testgen.ProjectTargetsRequest, Testgen.ProjectTargetsResponse>(request) {
    override val logMessage: String = "Sending request to get PROJECT TARGETS."
    override suspend fun Testgen.ProjectTargetsResponse.handle(cancellationJob: Job?) {
        processTargets(this)
    }

    override suspend fun TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Testgen.ProjectTargetsResponse {
        return getProjectTargets(request)
    }
}
