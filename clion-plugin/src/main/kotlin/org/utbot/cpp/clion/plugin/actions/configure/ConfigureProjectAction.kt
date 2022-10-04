package org.utbot.cpp.clion.plugin.actions.configure

import com.intellij.openapi.actionSystem.AnActionEvent
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.actions.UTBotBaseAction
import org.utbot.cpp.clion.plugin.client.requests.CheckProjectConfigurationRequest
import org.utbot.cpp.clion.plugin.utils.activeProject

class ConfigureProjectAction : UTBotBaseAction(UTBot.message("projectConfigure.configure")) {

    override fun actionPerformed(e: AnActionEvent) = CheckProjectConfigurationRequest(e.activeProject()).execute()

    override fun updateIfEnabled(e: AnActionEvent) {
        e.presentation.isEnabledAndVisible = e.project != null
    }
}
