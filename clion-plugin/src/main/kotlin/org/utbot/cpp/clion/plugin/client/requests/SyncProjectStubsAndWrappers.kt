package org.utbot.cpp.clion.plugin.client.requests

import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.collect
import kotlinx.coroutines.flow.onEach
import org.utbot.cpp.clion.plugin.client.handlers.SourceCode
import org.utbot.cpp.clion.plugin.grpc.GrpcRequestBuilder
import org.utbot.cpp.clion.plugin.utils.createFileWithText
import org.utbot.cpp.clion.plugin.utils.logger
import org.utbot.cpp.clion.plugin.utils.markDirtyAndRefresh
import org.utbot.cpp.clion.plugin.utils.nioPath
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt

class SyncProjectStubsAndWrappers(builder: GrpcRequestBuilder<Testgen.ProjectRequest>, project: Project) :
    BaseRequest<Testgen.ProjectRequest, Flow<Testgen.StubsResponse>>(builder, project) {
    override val logMessage: String
        get() = "Requesting project stubs and wrappers from server"
    override val id: String
        get() = "Sync project stubs and wrappers"

    override suspend fun Flow<Testgen.StubsResponse>.handle(cancellationJob: Job?) {
        this.onEach {
            if (it.stubSourcesCount > 0) {
                it.stubSourcesList.map { grpcSC ->
                    SourceCode(grpcSC, project).apply {
                        project.logger.trace { "Creating file $localPath" }
                        createFileWithText(localPath, content)
                    }
                }
            }
        }.collect()

        markDirtyAndRefresh(project.nioPath)
    }

    override suspend fun TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Flow<Testgen.StubsResponse> {
        return this.generateProjectStubs(request)
    }
}