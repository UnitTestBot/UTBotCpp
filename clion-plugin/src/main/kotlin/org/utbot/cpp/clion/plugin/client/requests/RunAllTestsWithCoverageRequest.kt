package org.utbot.cpp.clion.plugin.client.requests

import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.client.handlers.CoverageAndResultsHandler
import org.utbot.cpp.clion.plugin.grpc.Params
import testsgen.Testgen
import testsgen.Testgen.CoverageAndResultsRequest
import testsgen.TestsGenServiceGrpcKt

class RunAllTestsWithCoverageRequest(
    params: Params<CoverageAndResultsRequest>,
    project: Project,
) : BaseRequest<CoverageAndResultsRequest, Flow<Testgen.CoverageAndResultsResponse>>(params, project) {
    override val id: String = "Run All Tests with Coverage"
    override val logMessage: String = "Sending request to get tests run results and coverage"

    override suspend fun Flow<Testgen.CoverageAndResultsResponse>.handle(cancellationJob: Job?) {
        if (cancellationJob?.isActive == true) {
            CoverageAndResultsHandler(
                project,
                this,
                UTBot.message("requests.coverage.description.progress"),
                cancellationJob,
            ).handle()
        }
    }

    override suspend fun TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Flow<Testgen.CoverageAndResultsResponse> =
        createTestsCoverageAndResult(request)
}
