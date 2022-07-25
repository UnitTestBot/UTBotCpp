package org.utbot.cpp.clion.plugin.actions

import com.intellij.notification.Notification
import com.intellij.notification.NotificationAction
import com.intellij.openapi.actionSystem.AnActionEvent
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.grpc.getProjectConfigGrpcRequest
import org.utbot.cpp.clion.plugin.client.requests.CheckProjectConfigurationRequest
import testsgen.Testgen

class ReconfigureProjectAction: NotificationAction(UTBot.message("projectConfigure.reconfigure")) {
    override fun actionPerformed(e: AnActionEvent, notification: Notification) {
        actionPerformed(e)
    }

    override fun actionPerformed(e: AnActionEvent) {
        CheckProjectConfigurationRequest(
            e.project!!,
            getProjectConfigGrpcRequest(e.project!!, Testgen.ConfigMode.ALL),
        ).execute()
    }

    override fun update(e: AnActionEvent) {
        e.presentation.isEnabledAndVisible = e.project != null
    }
}