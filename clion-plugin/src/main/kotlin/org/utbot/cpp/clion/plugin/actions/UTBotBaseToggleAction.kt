package org.utbot.cpp.clion.plugin.actions

import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.ToggleAction
import javax.swing.Icon
import org.utbot.cpp.clion.plugin.utils.isPluginEnabled

abstract class UTBotBaseToggleAction(
    text: () -> String? = { null },
    description: () -> String? = { null },
    icon: Icon? = null
): ToggleAction(text, description, icon) {
    override fun update(e: AnActionEvent) {
        super.update(e)
        if (isPluginEnabled(e)) {
            updateIfEnabled(e)
        } else {
            e.presentation.isEnabledAndVisible = false
        }
    }

    abstract fun updateIfEnabled(e: AnActionEvent)
}