package com.huawei.utbot.cpp.actions

import com.huawei.utbot.cpp.UTBot
import com.huawei.utbot.cpp.actions.utils.getProjectConfigRequestMessage
import com.huawei.utbot.cpp.client.Requests.CheckProjectConfigurationRequest
import com.huawei.utbot.cpp.utils.execute
import com.intellij.notification.Notification
import com.intellij.notification.NotificationAction
import com.intellij.openapi.actionSystem.AnActionEvent
import testsgen.Testgen

class ConfigureProjectAction : NotificationAction(UTBot.message("projectConfigure.configure")) {
    override fun actionPerformed(e: AnActionEvent, n: Notification) {
        actionPerformed(e)
    }

    override fun actionPerformed(e: AnActionEvent) {
        CheckProjectConfigurationRequest(
            e.project!!,
            getProjectConfigRequestMessage(e.project!!, Testgen.ConfigMode.CHECK)
        ).execute(e)
    }

    override fun update(e: AnActionEvent) {
        e.presentation.isEnabledAndVisible = e.project != null
    }
}
