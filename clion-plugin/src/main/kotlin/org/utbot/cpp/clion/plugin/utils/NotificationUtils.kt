package org.utbot.cpp.clion.plugin.utils

import com.intellij.notification.NotificationGroupManager
import com.intellij.notification.NotificationType
import com.intellij.openapi.actionSystem.AnAction
import com.intellij.openapi.project.Project
import org.utbot.cpp.clion.plugin.UTBot
import testsgen.Testgen

fun notifyError(title: String, errorText: String, project: Project? = null, vararg actions: AnAction?) =
    notify(NotificationType.ERROR, title, errorText,project, *actions)

fun notifyInfo(title: String, infoText: String, project: Project? = null, vararg actions: AnAction?) =
    notify(NotificationType.INFORMATION, title, infoText, project, *actions)

fun notifyWarning(title: String, warningText: String, project: Project? = null, vararg actions: AnAction?) =
    notify(NotificationType.WARNING, title, warningText, project, *actions)

fun notifyUnknownResponse(response: Testgen.ProjectConfigResponse, project: Project) =
    notifyError(UTBot.message("notify.title.error"), "Unknown server response: ${response.message}", project)

private fun notify(
    type: NotificationType,
    title: String,
    content: String,
    project: Project? = null,
    vararg actions: AnAction?
) {
    val notification = NotificationGroupManager.getInstance()
        .getNotificationGroup("UTBot events")
        .createNotification(title, content, type)

    notification.notify(project)
    actions.filterNotNull().forEach {
        notification.addAction(it)
    }
}
