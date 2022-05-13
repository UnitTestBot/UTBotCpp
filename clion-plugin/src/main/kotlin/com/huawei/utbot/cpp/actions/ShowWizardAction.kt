package com.huawei.utbot.cpp.actions

import com.huawei.utbot.cpp.UTBot
import com.huawei.utbot.cpp.ui.wizard.UTBotWizard
import com.huawei.utbot.cpp.utils.utbotSettings
import com.intellij.notification.Notification
import com.intellij.notification.NotificationAction
import com.intellij.openapi.actionSystem.AnActionEvent

class ShowWizardAction: NotificationAction(UTBot.message("wizard.show")) {
    override fun actionPerformed(e: AnActionEvent, notification: Notification) {
        actionPerformed(e)
    }

    override fun actionPerformed(e: AnActionEvent) {
        if (UTBotWizard(e.project ?: return).showAndGet())
            // when user completes the wizard, some new settings were probably set
            e.project!!.utbotSettings.fireUTBotSettingsChanged()
    }

    override fun update(e: AnActionEvent) {
        e.presentation.isEnabled = e.project != null
    }
}
