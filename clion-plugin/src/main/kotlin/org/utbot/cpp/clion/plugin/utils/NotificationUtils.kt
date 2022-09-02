package org.utbot.cpp.clion.plugin.utils

import com.intellij.notification.NotificationGroupManager
import com.intellij.notification.NotificationType
import com.intellij.openapi.actionSystem.AnAction
import com.intellij.openapi.project.Project
import testsgen.Testgen

fun notifyError(errorText: String, project: Project? = null, action: AnAction? = null) =
    notify(NotificationType.ERROR, errorText, project, action)

fun notifyInfo(infoText: String, project: Project? = null, action: AnAction? = null) =
    notify(NotificationType.INFORMATION, infoText, project, action)

fun notifyWarning(warningText: String, project: Project? = null, action: AnAction? = null) =
    notify(NotificationType.WARNING, warningText, project, action)

fun notifyUnknownResponse(response: Testgen.ProjectConfigResponse, project: Project) =
    notifyInfo("Unknown server response: ${response.message}", project)

private fun notify(
    type: NotificationType,
    content: String,
    project: Project? = null,
    action: AnAction? = null,
) {
    val notification = NotificationGroupManager.getInstance()
        .getNotificationGroup("UTBot events")
        .createNotification(content, type)

    notification.notify(project)
    action?.let { notification.addAction(it) }
}
