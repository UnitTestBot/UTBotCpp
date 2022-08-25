package org.utbot.cpp.clion.plugin.actions

import com.intellij.openapi.actionSystem.AnAction
import com.intellij.openapi.actionSystem.AnActionEvent
import javax.swing.Icon
import org.utbot.cpp.clion.plugin.settings.settings

abstract class UTBotBaseAction(
    text: () -> String? = { null },
    description: () -> String? = { null },
    icon: Icon? = null
) : AnAction(text, description, icon) {
    constructor(text: String) : this({ text })

    override fun update(e: AnActionEvent) {
        var isEnabled = false
        e.project?.let {
            isEnabled = it.settings.storedSettings.isPluginEnabled
        }
        e.presentation.isEnabledAndVisible = isEnabled
        if (isEnabled) {
            updateIfEnabled(e)
        }
    }

    abstract fun updateIfEnabled(e: AnActionEvent)
}
