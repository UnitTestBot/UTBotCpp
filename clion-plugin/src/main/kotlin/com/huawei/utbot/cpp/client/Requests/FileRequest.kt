package com.huawei.utbot.cpp.client.Requests

import com.huawei.utbot.cpp.UTBot
import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt

class FileRequest(
    request: Testgen.FileRequest,
    project: Project,
) : BaseTestsRequest<Testgen.FileRequest>(request, project, UTBot.message("requests.file.description.progress")) {
    override val logMessage: String = "Sending request to generate for FILE."
    override suspend fun TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Flow<Testgen.TestsResponse> =
        generateFileTests(request)
}
