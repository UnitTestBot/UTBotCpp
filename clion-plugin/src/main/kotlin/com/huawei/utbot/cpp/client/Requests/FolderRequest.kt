package com.huawei.utbot.cpp.client.Requests

import com.huawei.utbot.cpp.UTBot
import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt

class FolderRequest(
    request: Testgen.FolderRequest,
    project: Project,
) : BaseTestsRequest<Testgen.FolderRequest>(request, project, UTBot.message("requests.folder.description.progress")) {
    override val target: String = "Folder"
    override val logMessage: String = "Sending request to generate tests for FOLDER."
    override suspend fun TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Flow<Testgen.TestsResponse> =
        generateFolderTests(request)
}
