package org.utbot.cpp.clion.plugin.client.requests

import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.client.handlers.CoverageAndResultsHandler
import org.utbot.cpp.clion.plugin.grpc.getRunWithCoverageRequestForAllTests
import org.utbot.cpp.clion.plugin.utils.activeProject
import testsgen.Testgen
import testsgen.Testgen.CoverageAndResultsRequest
import testsgen.TestsGenServiceGrpcKt

class RunAllTestsWithCoverageRequest(
    request: CoverageAndResultsRequest,
    project: Project,
) : BaseRequest<CoverageAndResultsRequest, Flow<Testgen.CoverageAndResultsResponse>>(request, project) {

    override val logMessage: String = "Sending request to get tests run results and coverage"

    constructor(e: AnActionEvent) : this(getRunWithCoverageRequestForAllTests(e.activeProject()), e.activeProject())

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
