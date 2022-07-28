package org.utbot.cpp.clion.plugin.actions.generate

import com.intellij.openapi.actionSystem.AnActionEvent
import org.utbot.cpp.clion.plugin.grpc.getProjectGrpcRequest
import org.utbot.cpp.clion.plugin.client.requests.ProjectRequest
import org.utbot.cpp.clion.plugin.utils.activeProject
import org.utbot.cpp.clion.plugin.utils.currentClient

class GenerateForProjectAction : BaseGenerateTestsAction() {
    override fun actionPerformed(e: AnActionEvent) {
        ProjectRequest(
            getProjectGrpcRequest(e),
            e.activeProject(),
        ).apply {
            e.currentClient.executeRequestIfNotDisposed(this)
        }
    }

    override fun isDefined(e: AnActionEvent): Boolean = e.project != null
}
