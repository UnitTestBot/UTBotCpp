package com.huawei.utbot.cpp.client.Requests

import com.huawei.utbot.cpp.UTBot
import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt

class PredicateRequest(
    request: Testgen.PredicateRequest,
    project: Project,
) : BaseTestsRequest<Testgen.PredicateRequest>(request, project, UTBot.message("requests.predicate.description.progress")) {
    override val logMessage: String = "Sending request to generate for LINE."
    override suspend fun TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Flow<Testgen.TestsResponse> =
        generatePredicateTests(request)
}