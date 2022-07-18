package org.utbot.cpp.clion.plugin.actions

import com.intellij.notification.Notification
import com.intellij.notification.NotificationAction
import com.intellij.openapi.actionSystem.AnActionEvent
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.client.requests.CreateBuildDirRequest

class AskServerToGenerateBuildDir : NotificationAction(UTBot.message("projectConfigure.generate.buildDir")) {
    override fun actionPerformed(e: AnActionEvent, n: Notification) {
        actionPerformed(e)
    }

    // todo: add constructor that takes AnActionEvent to Request classes
    override fun actionPerformed(e: AnActionEvent) {
        CreateBuildDirRequest(e).execute()
    }

    override fun update(e: AnActionEvent) {
        e.presentation.isEnabledAndVisible = e.project != null
    }
}
