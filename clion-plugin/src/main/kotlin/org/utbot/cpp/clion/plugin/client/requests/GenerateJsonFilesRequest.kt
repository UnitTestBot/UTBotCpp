package org.utbot.cpp.clion.plugin.client.requests

import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.client.handlers.GenerateJsonHandler
import org.utbot.cpp.clion.plugin.grpc.GrpcRequestBuilder
import org.utbot.cpp.clion.plugin.grpc.GrpcRequestBuilderFactory
import org.utbot.cpp.clion.plugin.utils.activeProject
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt

class GenerateJsonFilesRequest(
    params: GrpcRequestBuilder<Testgen.ProjectConfigRequest>,
    project: Project
) : BaseRequest<Testgen.ProjectConfigRequest, Flow<Testgen.ProjectConfigResponse>>(params, project) {
    override val id: String = "Generate JSON Files"
    override val logMessage: String = "Sending request to check project configuration."

    constructor(project: Project) : this(
        GrpcRequestBuilderFactory(project).createProjectConfigRequestBuilder(Testgen.ConfigMode.GENERATE_JSON_FILES),
        project
    )

    constructor(e: AnActionEvent) : this(e.activeProject())

    override suspend fun TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Flow<Testgen.ProjectConfigResponse> {
        return this.configureProject(request)
    }

    override suspend fun Flow<Testgen.ProjectConfigResponse>.handle(cancellationJob: Job?) {
        if (cancellationJob?.isActive == true) {
            GenerateJsonHandler(
                project,
                this,
                UTBot.message("requests.json.description.progress"),
                cancellationJob
            ).handle()
        }
    }
}
