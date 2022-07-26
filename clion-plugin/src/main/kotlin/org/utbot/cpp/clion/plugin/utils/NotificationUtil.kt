package org.utbot.cpp.clion.plugin.utils

import com.intellij.notification.NotificationGroupManager
import com.intellij.notification.NotificationType
import com.intellij.openapi.actionSystem.AnAction
import com.intellij.openapi.project.Project
import testsgen.Testgen

fun notifyError(content: String, project: Project? = null, action: AnAction? = null) {
    notify(content, NotificationType.ERROR, project, action)
}

fun notifyInfo(content: String, project: Project? = null, action: AnAction? = null) {
    notify(content, NotificationType.INFORMATION, project, action)
}

fun notifyWarning(content: String, project: Project? = null, action: AnAction? = null) {
    notify(content, NotificationType.WARNING, project, action)
}

fun notifyProjectPathUnset() {
    // todo: make hyperlink to settings
    notifyWarning("Could not guess project directory! Please specify it in settings.")
}

fun notify(content: String, type: NotificationType, project: Project? = null, action: AnAction? = null) {
    val notification = NotificationGroupManager.getInstance().getNotificationGroup("Custom Notification Group")
        .createNotification(content, type)
    if (action != null)
        notification.addAction(action)
    notification.notify(project)
}

fun notifyUnknownResponse(response: Testgen.ProjectConfigResponse, project: Project? = null) =
    notifyInfo("Unknown server response: ${response.message}", project)
