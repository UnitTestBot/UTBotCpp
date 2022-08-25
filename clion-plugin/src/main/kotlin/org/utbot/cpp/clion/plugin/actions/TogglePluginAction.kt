package org.utbot.cpp.clion.plugin.actions

import com.intellij.openapi.actionSystem.ActionPlaces
import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.ToggleAction
import com.intellij.openapi.project.Project
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.settings.settings

class TogglePluginAction : ToggleAction() {
    override fun isSelected(e: AnActionEvent): Boolean {
        val project = e.project ?: return false
        updateActionText(e, project)
        return project.settings.storedSettings.isPluginEnabled
    }

    override fun setSelected(e: AnActionEvent, state: Boolean) {
        val project = e.project ?: return
        val previousValue = project.settings.storedSettings.isPluginEnabled
        project.settings.storedSettings.isPluginEnabled = state
        updateActionText(e, project)
        if (previousValue != state)
            project.settings.fireUTBotEnabledStateChanged()
    }

    private fun updateActionText(e: AnActionEvent, project: Project) {
        val isPluginEnabled = project.settings.storedSettings.isPluginEnabled
        var newText = if (isPluginEnabled) UTBot.message("actions.enable.enabled")
        else UTBot.message("actions.enable.disabled")
        if (ActionPlaces.isPopupPlace(e.place)) {
            newText = if (isPluginEnabled)
                UTBot.message("actions.enable.menu.enabled")
            else UTBot.message("actions.enable.menu.disabled")
        }
        e.presentation.text = newText
    }
}
