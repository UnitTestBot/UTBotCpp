package org.utbot.cpp.clion.plugin.client.requests

import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import org.utbot.cpp.clion.plugin.UTBot
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub

class ProjectRequest(
    request: Testgen.ProjectRequest,
    project: Project,
) : BaseTestsRequest<Testgen.ProjectRequest>(request, project, UTBot.message("requests.project.description.progress")) {

    override val logMessage: String = "Sending request to generate tests for project"
    override fun getInfoMessage(): String = "Tests for project are generated!"

    override suspend fun TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Flow<Testgen.TestsResponse> =
        generateProjectTests(request)
}
