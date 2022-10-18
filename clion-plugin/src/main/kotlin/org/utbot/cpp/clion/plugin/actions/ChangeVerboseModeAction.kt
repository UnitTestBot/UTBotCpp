package org.utbot.cpp.clion.plugin.actions

import com.intellij.openapi.actionSystem.ActionPlaces
import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.CommonDataKeys
import com.intellij.openapi.project.Project
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.settings.settings

class ChangeVerboseModeAction : UTBotBaseToggleAction() {
    override fun isSelected(e: AnActionEvent): Boolean {
        updateActionText(e)
        return e.getRequiredData(CommonDataKeys.PROJECT).settings.storedSettings.verbose
    }

    override fun isDumbAware(): Boolean = true

    override fun updateIfEnabled(e: AnActionEvent) {
        e.presentation.isEnabledAndVisible = (e.project != null)
    }

    override fun setSelected(e: AnActionEvent, state: Boolean) {
        with(e.getRequiredData(CommonDataKeys.PROJECT).settings) {
            storedSettings.verbose = state
            fireUTBotSettingsChanged()
        }
        updateActionText(e)
    }

    private fun updateActionText(e: AnActionEvent) {
        e.presentation.text = getActionText(e.getRequiredData(CommonDataKeys.PROJECT), e.place)
    }

    companion object {
        private fun getActionText(project: Project, place: String? = null): String {
            val isVerboseEnabled = project.settings.storedSettings.verbose
            var newText = if (isVerboseEnabled) UTBot.message("actions.verbose.enabled")
            else UTBot.message("actions.verbose.disabled")
            if (place != null && ActionPlaces.isPopupPlace(place)) {
                newText = if (isVerboseEnabled)
                    UTBot.message("actions.verbose.menu.enabled")
                else UTBot.message("actions.verbose.menu.disabled")
            }
            return newText
        }
    }
}
