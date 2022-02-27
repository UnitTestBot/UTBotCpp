package actions

import actions.utils.client
import com.intellij.notification.Notification
import com.intellij.notification.NotificationAction
import com.intellij.openapi.actionSystem.AnActionEvent

class ConfigureProjectAction : NotificationAction(UTBot.message("projectConfigure.configure")) {
    override fun actionPerformed(e: AnActionEvent, n: Notification) {
        actionPerformed(e)
    }

    override fun actionPerformed(e: AnActionEvent) {
        e.client.configureProject()
    }

    override fun update(e: AnActionEvent) {
        e.presentation.isEnabledAndVisible = e.project != null
    }
}
