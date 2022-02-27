package org.utbot.cpp.clion.plugin.actions

import com.intellij.notification.Notification
import com.intellij.notification.NotificationAction
import com.intellij.openapi.actionSystem.AnActionEvent
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.ui.wizard.UTBotWizard

class ShowWizardAction: NotificationAction(UTBot.message("wizard.show")) {
    override fun actionPerformed(e: AnActionEvent, notification: Notification) {
        actionPerformed(e)
    }

    override fun actionPerformed(e: AnActionEvent) {
        UTBotWizard(e.project ?: return).showAndGet()
    }

    override fun update(e: AnActionEvent) {
        e.presentation.isEnabled = e.project != null
    }
}
