package org.utbot.cpp.clion.plugin.actions.configure

import com.intellij.openapi.actionSystem.AnAction
import com.intellij.openapi.actionSystem.AnActionEvent
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.grpc.getProjectConfigGrpcRequest
import org.utbot.cpp.clion.plugin.client.requests.CheckProjectConfigurationRequest
import org.utbot.cpp.clion.plugin.utils.activeProject
import testsgen.Testgen

class ReconfigureProjectAction: AnAction(UTBot.message("projectConfigure.reconfigure")) {

    override fun actionPerformed(e: AnActionEvent) {
        val project = e.activeProject()
        CheckProjectConfigurationRequest(
            getProjectConfigGrpcRequest(project, Testgen.ConfigMode.ALL),
            project,
        ).execute()
    }

    override fun update(e: AnActionEvent) {
        e.presentation.isEnabledAndVisible = e.project != null
    }
}