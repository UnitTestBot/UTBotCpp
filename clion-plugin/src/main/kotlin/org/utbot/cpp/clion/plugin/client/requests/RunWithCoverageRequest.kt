package org.utbot.cpp.clion.plugin.client.requests

import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.client.handlers.CoverageAndResultsHandler
import org.utbot.cpp.clion.plugin.utils.convertFromRemotePathIfNeeded
import org.utbot.cpp.clion.plugin.utils.testFilePathToSourceFilePath
import testsgen.Testgen
import testsgen.Testgen.CoverageAndResultsResponse
import testsgen.TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub

class RunWithCoverageRequest(
    request: Testgen.CoverageAndResultsRequest,
    project: Project,
): BaseRequest<Testgen.CoverageAndResultsRequest, Flow<CoverageAndResultsResponse>>(request, project) {

    override val logMessage: String = "Sending request to get tests run results and coverage"

    override suspend fun Flow<CoverageAndResultsResponse>.handle(cancellationJob: Job?) {
        //TODO: I do not understand this condition here
        if (cancellationJob?.isActive == true) {
            CoverageAndResultsHandler(
                project,
                this,
                UTBot.message("requests.coverage.description.progress"),
                cancellationJob,
                testFilePathToSourceFilePath(request.testFilter.testFilePath.convertFromRemotePathIfNeeded(project), project)
            ).handle()
        }
    }

    override suspend fun TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Flow<CoverageAndResultsResponse> =
        createTestsCoverageAndResult(request)
}
