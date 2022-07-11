package com.huawei.utbot.cpp.utils

import com.huawei.utbot.cpp.client.Client
import com.huawei.utbot.cpp.client.ClientManager
import com.huawei.utbot.cpp.client.Request
import com.huawei.utbot.cpp.client.logger.ClientLogger
import com.huawei.utbot.cpp.services.GeneratorSettings
import com.huawei.utbot.cpp.services.UTBotSettings
import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.CommonDataKeys
import com.intellij.openapi.application.ApplicationManager
import com.intellij.openapi.components.service
import com.intellij.openapi.project.Project
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.job

val Project.logger: ClientLogger
    get() = this.service<ClientLogger>()

val AnActionEvent.client: Client
    get() = this.getRequiredData(CommonDataKeys.PROJECT).getClient()

fun Project.getClient(): Client = this.service<ClientManager>().client

val Project.utbotSettings: UTBotSettings
    get() = this.service()

val Project.generatorSettings: GeneratorSettings
    get() = this.service()

fun Request.execute(e: AnActionEvent) {
    e.client.executeRequest(this)
}

fun String.convertFromRemotePathIfNeeded(project: Project): String {
    return project.service<UTBotSettings>().convertFromRemotePathIfNeeded(this)
}

fun String.convertToRemotePathIfNeeded(project: Project): String {
    return project.service<UTBotSettings>().convertToRemotePathIfNeeded(this)
}

val CoroutineScope.children
    get() = this.coroutineContext.job.children.toList()

fun CoroutineScope.hasChildren(): Boolean = children.isNotEmpty()

fun invokeOnEdt(task: ()->Unit) {
    ApplicationManager.getApplication().invokeLater(task)
}

fun isWindows() = System.getProperty("os.name").contains("win", ignoreCase = true)
