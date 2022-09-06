package org.utbot.cpp.clion.plugin.actions

import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.components.service
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.client.ManagedClient

class ReconnectAction : UTBotBaseAction(UTBot.message("actions.reconnect")) {
    override fun actionPerformed(e: AnActionEvent) {
        e.project!!.service<ManagedClient>().restartClient()
    }

    override fun updateIfEnabled(e: AnActionEvent) {
        e.presentation.isEnabledAndVisible = e.project != null
    }
}
