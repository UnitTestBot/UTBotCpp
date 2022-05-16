package com.huawei.utbot.cpp.actions

import com.huawei.utbot.cpp.UTBot
import com.huawei.utbot.cpp.actions.utils.getProjectConfigRequestMessage
import com.huawei.utbot.cpp.client.requests.CreateBuildDirRequest
import com.huawei.utbot.cpp.utils.execute
import com.intellij.notification.Notification
import com.intellij.notification.NotificationAction
import com.intellij.openapi.actionSystem.AnActionEvent
import testsgen.Testgen

class AskServerToGenerateBuildDir : NotificationAction(UTBot.message("projectConfigure.generate.buildDir")) {
    override fun actionPerformed(e: AnActionEvent, n: Notification) {
        actionPerformed(e)
    }

    // todo: add constructor that takes AnActionEvent to Request classes
    override fun actionPerformed(e: AnActionEvent) {
        CreateBuildDirRequest(
            e.project!!,
            getProjectConfigRequestMessage(e.project!!, Testgen.ConfigMode.CREATE_BUILD_DIR)
        ).execute(e)
    }

    override fun update(e: AnActionEvent) {
        e.presentation.isEnabledAndVisible = e.project != null
    }
}
