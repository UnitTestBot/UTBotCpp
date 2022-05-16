package com.huawei.utbot.cpp.client.requests

import com.huawei.utbot.cpp.UTBot
import com.huawei.utbot.cpp.client.handlers.CoverageAndResultsHandler
import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt

class RunWithCoverageRequest(
    val project: Project,
    request: Testgen.CoverageAndResultsRequest
): BaseRequest<Testgen.CoverageAndResultsRequest, Flow<Testgen.CoverageAndResultsResponse>>(request) {
    override val logMessage: String = "Sending request to get tests RESULTS and COVERAGE."

    override suspend fun Flow<Testgen.CoverageAndResultsResponse>.handle(cancellationJob: Job?) {
        if (cancellationJob?.isActive == true) {
            CoverageAndResultsHandler(
                project,
                this,
                UTBot.message("requests.coverage.description.progress"),
                cancellationJob
            ).handle()
        }
    }

    override suspend fun TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Flow<Testgen.CoverageAndResultsResponse> {
        return createTestsCoverageAndResult(request)
    }
}
