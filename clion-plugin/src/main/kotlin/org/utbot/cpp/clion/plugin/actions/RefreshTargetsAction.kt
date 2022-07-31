package org.utbot.cpp.clion.plugin.actions

import com.intellij.icons.AllIcons
import com.intellij.openapi.actionSystem.AnAction
import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.components.service
import org.utbot.cpp.clion.plugin.grpc.activeProject
import org.utbot.cpp.clion.plugin.ui.targetsToolWindow.UTBotTargetsController

class RefreshTargetsAction: AnAction() {
    override fun actionPerformed(e: AnActionEvent) {
        e.activeProject().service<UTBotTargetsController>().requestTargetsFromServer()
    }

    override fun update(e: AnActionEvent) {
        e.presentation.isEnabledAndVisible = e.project != null
        e.presentation.icon = AllIcons.Actions.Refresh
    }
}