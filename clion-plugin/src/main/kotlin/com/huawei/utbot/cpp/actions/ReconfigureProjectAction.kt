package com.huawei.utbot.cpp.actions

import com.huawei.utbot.cpp.UTBot
import com.huawei.utbot.cpp.actions.utils.getProjectConfigRequestMessage
import com.huawei.utbot.cpp.client.Requests.CheckProjectConfigurationRequest
import com.huawei.utbot.cpp.utils.execute
import com.intellij.notification.Notification
import com.intellij.notification.NotificationAction
import com.intellij.openapi.actionSystem.AnActionEvent
import testsgen.Testgen

class ReconfigureProjectAction: NotificationAction(UTBot.message("projectConfigure.reconfigure")) {
    override fun actionPerformed(e: AnActionEvent, notification: Notification) {
        actionPerformed(e)
    }

    override fun actionPerformed(e: AnActionEvent) {
        CheckProjectConfigurationRequest(
            e.project!!,
            getProjectConfigRequestMessage(e.project!!, Testgen.ConfigMode.ALL),
        ).execute(e)
    }

    override fun update(e: AnActionEvent) {
        e.presentation.isEnabledAndVisible = e.project != null
    }
}