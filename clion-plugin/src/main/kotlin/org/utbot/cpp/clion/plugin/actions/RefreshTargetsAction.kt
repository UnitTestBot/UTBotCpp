package org.utbot.cpp.clion.plugin.actions

import com.intellij.icons.AllIcons
import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.components.service
import org.utbot.cpp.clion.plugin.ui.utbotToolWindow.targetToolWindow.UTBotTargetsController
import org.utbot.cpp.clion.plugin.utils.activeProject

class RefreshTargetsAction: UTBotBaseAction() {
    override fun actionPerformed(e: AnActionEvent) {
        e.activeProject().service<UTBotTargetsController>().requestTargetsFromServer()
    }

    override fun updateIfEnabled(e: AnActionEvent) {
        e.presentation.isEnabledAndVisible = e.project != null
        e.presentation.icon = AllIcons.Actions.Refresh
    }
}
