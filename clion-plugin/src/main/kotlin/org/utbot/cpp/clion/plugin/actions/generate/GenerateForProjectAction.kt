package org.utbot.cpp.clion.plugin.actions.generate

import com.intellij.openapi.actionSystem.AnActionEvent
import org.utbot.cpp.clion.plugin.client.requests.test.ProjectRequest
import org.utbot.cpp.clion.plugin.grpc.ParamsBuilder
import org.utbot.cpp.clion.plugin.utils.activeProject
import org.utbot.cpp.clion.plugin.utils.client

class GenerateForProjectAction : BaseGenerateTestsAction() {
    override fun actionPerformed(e: AnActionEvent) {
        ProjectRequest(
            ParamsBuilder(e.activeProject()).buildProjectRequestParams(),
            e.activeProject(),
        ).apply {
            e.client.executeRequest(this)
        }
    }

    override fun isDefined(e: AnActionEvent): Boolean = e.project != null
}
