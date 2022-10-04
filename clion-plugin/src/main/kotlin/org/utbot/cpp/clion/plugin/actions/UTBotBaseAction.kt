package org.utbot.cpp.clion.plugin.actions

import com.intellij.openapi.actionSystem.AnAction
import com.intellij.openapi.actionSystem.AnActionEvent
import javax.swing.Icon
import org.utbot.cpp.clion.plugin.utils.isPluginEnabled

abstract class UTBotBaseAction(
    text: () -> String? = { null },
    description: () -> String? = { null },
    icon: Icon? = null
) : AnAction(text, description, icon) {
    constructor(text: String) : this({ text })

    override fun update(e: AnActionEvent) {
        if (isPluginEnabled(e)) {
            updateIfEnabled(e)
        } else {
            e.presentation.isEnabledAndVisible = false
        }
    }

    abstract fun updateIfEnabled(e: AnActionEvent)
}
