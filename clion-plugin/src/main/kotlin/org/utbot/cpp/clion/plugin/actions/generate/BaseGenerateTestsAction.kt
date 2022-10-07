package org.utbot.cpp.clion.plugin.actions.generate

import com.intellij.openapi.actionSystem.AnActionEvent
import org.utbot.cpp.clion.plugin.actions.UTBotBaseAction
import org.utbot.cpp.clion.plugin.client.ManagedClient
import org.utbot.cpp.clion.plugin.utils.activeProject
import org.utbot.cpp.clion.plugin.utils.client

abstract class BaseGenerateTestsAction : UTBotBaseAction() {

    override fun updateIfEnabled(e: AnActionEvent) {
        val isDefined: Boolean = isDefined(e)

        e.presentation.isVisible = isDefined
        e.presentation.isEnabled = isDefined && ManagedClient.isConnectedToServer(e.activeProject())
    }

    /**
     * Checks if an action can really be called for event [e].
     * For example, editor must be initialized for all actions, related to
     * specific lines, and must not for related to whole project.
     */
    abstract fun isDefined(e: AnActionEvent): Boolean
}
