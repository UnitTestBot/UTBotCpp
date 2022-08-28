package org.utbot.cpp.clion.plugin.actions

import com.intellij.openapi.actionSystem.AnActionEvent
import org.utbot.cpp.clion.plugin.utils.client

class SyncWrappersAndStubsAction: UTBotBaseAction() {
    override fun actionPerformed(e: AnActionEvent) {
        e.client.syncWrappersAnsStubs()
    }

    override fun updateIfEnabled(e: AnActionEvent) {
        e.presentation.isEnabledAndVisible = e.project != null
    }
}
