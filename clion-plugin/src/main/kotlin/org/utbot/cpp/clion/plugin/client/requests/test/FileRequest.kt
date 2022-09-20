package org.utbot.cpp.clion.plugin.client.requests.test

import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.grpc.Params
import org.utbot.cpp.clion.plugin.utils.fileNameOrNull
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub

class FileRequest(
    params: Params<Testgen.FileRequest>,
    project: Project,
) : BaseTestsRequest<Testgen.FileRequest>(params, project, UTBot.message("requests.file.description.progress")) {
    override val id: String = "Generate for File"

    override val logMessage: String = "Sending request to generate tests for file"
    override fun getInfoMessage(): String = "Tests for file <em>${(request.filePath.fileNameOrNull()?.plus(" ")) ?: ""}</em> are generated!"

    override suspend fun TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Flow<Testgen.TestsResponse> =
        generateFileTests(request)
}
