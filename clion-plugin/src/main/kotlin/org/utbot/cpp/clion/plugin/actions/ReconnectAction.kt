package org.utbot.cpp.clion.plugin.actions

import com.intellij.openapi.actionSystem.AnAction
import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.components.service
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.client.ClientManager

class ReconnectAction: AnAction(UTBot.message("actions.reconnect")) {
    override fun actionPerformed(e: AnActionEvent) {
        e.project!!.service<ClientManager>().restartClient()
    }

    override fun update(e: AnActionEvent) {
        e.presentation.isEnabledAndVisible = e.project != null
    }
}
