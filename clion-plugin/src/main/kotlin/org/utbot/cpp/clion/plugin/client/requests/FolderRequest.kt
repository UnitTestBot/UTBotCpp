package org.utbot.cpp.clion.plugin.client.requests

import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.utils.fileNameOrNull
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt

class FolderRequest(
    request: Testgen.FolderRequest,
    project: Project,
) : BaseTestsRequest<Testgen.FolderRequest>(request, project, UTBot.message("requests.folder.description.progress")) {
    override val logMessage: String = "Sending request to generate tests for FOLDER."
    override fun getInfoMessage(): String = "Tests for folder <em>${request.folderPath.fileNameOrNull()?.plus(" ") ?: ""}</em> generated!"
    override suspend fun TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Flow<Testgen.TestsResponse> =
        generateFolderTests(request)
}
