package org.utbot.cpp.clion.plugin.actions

import com.intellij.notification.Notification
import com.intellij.notification.NotificationAction
import com.intellij.openapi.actionSystem.AnActionEvent
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.client.requests.CheckProjectConfigurationRequest

class ConfigureProjectAction : NotificationAction(UTBot.message("projectConfigure.configure")) {
    override fun actionPerformed(e: AnActionEvent, n: Notification) {
        actionPerformed(e)
    }

    override fun actionPerformed(e: AnActionEvent) {
        CheckProjectConfigurationRequest(e).execute()
    }

    override fun update(e: AnActionEvent) {
        e.presentation.isEnabledAndVisible = e.project != null
    }
}
