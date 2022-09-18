package org.utbot.cpp.clion.plugin.client.requests.test

import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.grpc.Params
import org.utbot.cpp.clion.plugin.utils.fileNameOrNull
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub

class FolderRequest(
    folderRequestParams: Params<Testgen.FolderRequest>,
    project: Project,
) : BaseTestsRequest<Testgen.FolderRequest>(folderRequestParams, project, UTBot.message("requests.folder.description.progress")) {

    override val logMessage: String = "Sending request to generate tests for folder."
    override fun getInfoMessage(): String = "Tests for folder <em>${request.folderPath.fileNameOrNull()?.plus(" ") ?: ""}</em> are generated!"

    override suspend fun TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Flow<Testgen.TestsResponse> =
        generateFolderTests(request)
}
