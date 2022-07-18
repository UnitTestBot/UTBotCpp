package org.utbot.cpp.clion.plugin.client.requests

import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.client.handlers.CoverageAndResultsHandler
import org.utbot.cpp.clion.plugin.utils.convertFromRemotePathIfNeeded
import org.utbot.cpp.clion.plugin.utils.testFilePathToSourceFilePath
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt

class RunWithCoverageRequest(
    project: Project,
    request: Testgen.CoverageAndResultsRequest
): BaseRequest<Testgen.CoverageAndResultsRequest, Flow<Testgen.CoverageAndResultsResponse>>(request, project) {
    override val logMessage: String = "Sending request to get tests RESULTS and COVERAGE."

    override suspend fun Flow<Testgen.CoverageAndResultsResponse>.handle(cancellationJob: Job?) {
        request.testFilter.testFilePath
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

    override suspend fun TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Flow<Testgen.CoverageAndResultsResponse> {
        return createTestsCoverageAndResult(request)
    }
}
