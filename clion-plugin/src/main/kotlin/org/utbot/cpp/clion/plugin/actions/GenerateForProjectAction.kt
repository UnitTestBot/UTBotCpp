package org.utbot.cpp.clion.plugin.actions

import com.intellij.openapi.actionSystem.AnActionEvent
import org.utbot.cpp.clion.plugin.grpc.getProjectRequest
import org.utbot.cpp.clion.plugin.client.requests.ProjectRequest
import org.utbot.cpp.clion.plugin.utils.client

class GenerateForProjectAction : GenerateTestsBaseAction() {
    override fun updateIfServerAvailable(e: AnActionEvent) {
        e.presentation.isEnabled = (e.project != null)
    }

    override fun actionPerformed(e: AnActionEvent) {
        ProjectRequest(
            getProjectRequest(e),
            e.project!!
        ).apply {
            e.client.executeRequest(this)
        }
    }
}
