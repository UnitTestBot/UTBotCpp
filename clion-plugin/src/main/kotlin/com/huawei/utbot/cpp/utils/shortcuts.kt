package com.huawei.utbot.cpp.utils

import com.huawei.utbot.cpp.client.Client
import com.huawei.utbot.cpp.client.Request
import com.huawei.utbot.cpp.services.GeneratorSettings
import com.huawei.utbot.cpp.services.UTBotSettings
import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.CommonDataKeys
import com.intellij.openapi.application.ApplicationManager
import com.intellij.openapi.components.service
import com.intellij.openapi.project.Project
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.job

val AnActionEvent.client: Client
    get() = this.getRequiredData(CommonDataKeys.PROJECT).service()

val Project.utbotSettings: UTBotSettings
    get() = this.service()

val Project.generatorSettings: GeneratorSettings
    get() = this.service()

fun Request.execute(e: AnActionEvent) {
    e.client.execute(this)
}

val Project.client: Client
    get() = this.service()

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
