package org.utbot.cpp.clion.plugin.actions.configure

import com.intellij.openapi.actionSystem.AnActionEvent
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.client.requests.CheckProjectConfigurationRequest
import org.utbot.cpp.clion.plugin.actions.UTBotBaseAction
import org.utbot.cpp.clion.plugin.grpc.GrpcRequestBuilderFactory
import org.utbot.cpp.clion.plugin.utils.activeProject
import testsgen.Testgen

class ReconfigureProjectAction: UTBotBaseAction(UTBot.message("projectConfigure.reconfigure")) {

    override fun actionPerformed(e: AnActionEvent) {
        val project = e.activeProject()
        CheckProjectConfigurationRequest(
            GrpcRequestBuilderFactory(project).createProjectConfigRequestBuilder(Testgen.ConfigMode.ALL),
            project,
        ).execute()
    }

    override fun updateIfEnabled(e: AnActionEvent) {
        e.presentation.isEnabledAndVisible = e.project != null
    }
}
