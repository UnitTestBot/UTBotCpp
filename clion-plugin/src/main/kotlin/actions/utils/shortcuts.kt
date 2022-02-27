package actions.utils

import client.Client
import services.UTBotSettings
import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.CommonDataKeys
import com.intellij.openapi.components.service
import com.intellij.openapi.project.Project

val AnActionEvent.client: Client
    get() = this.getRequiredData(CommonDataKeys.PROJECT).service()

fun String.convertFromRemotePathIfNeeded(project: Project): String {
    return project.service<UTBotSettings>().convertFromRemotePathIfNeeded(this)
}

fun String.convertToRemotePathIfNeeded(project: Project): String {
    return project.service<UTBotSettings>().convertToRemotePathIfNeeded(this)
}
