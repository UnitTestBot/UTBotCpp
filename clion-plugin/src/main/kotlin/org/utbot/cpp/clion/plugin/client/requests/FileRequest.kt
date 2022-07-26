package org.utbot.cpp.clion.plugin.client.requests

import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.utils.fileNameOrNull
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt

class FileRequest(
    request: Testgen.FileRequest,
    project: Project,
) : BaseTestsRequest<Testgen.FileRequest>(request, project, UTBot.message("requests.file.description.progress")) {
    override val logMessage: String = "Sending request to generate for FILE."
    override fun getInfoMessage(): String = "Tests for file <em>${(request.filePath.fileNameOrNull()?.plus(" ")) ?: ""}</em>generated!"
    override suspend fun TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Flow<Testgen.TestsResponse> =
        generateFileTests(request)
}
