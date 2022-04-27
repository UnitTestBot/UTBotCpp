package com.huawei.utbot.cpp.actions

import com.huawei.utbot.cpp.UTBot
import com.huawei.utbot.cpp.actions.utils.getProjectConfigRequestMessage
import com.huawei.utbot.cpp.client.Requests.GenerateJsonFilesRequest
import com.huawei.utbot.cpp.utils.execute
import com.intellij.notification.Notification
import com.intellij.notification.NotificationAction
import com.intellij.openapi.actionSystem.AnActionEvent
import testsgen.Testgen

class AskServerToGenerateJsonForProjectConfiguration :
    NotificationAction(UTBot.message("projectConfigure.generate.json")) {
    override fun actionPerformed(e: AnActionEvent, n: Notification) {
        actionPerformed(e)
    }

    override fun actionPerformed(e: AnActionEvent) {
        GenerateJsonFilesRequest(
            e.project!!,
            getProjectConfigRequestMessage(e.project!!, Testgen.ConfigMode.GENERATE_JSON_FILES)
        ).execute(e)
    }

    override fun update(e: AnActionEvent) {
        e.presentation.isEnabledAndVisible = e.project != null
    }
}
