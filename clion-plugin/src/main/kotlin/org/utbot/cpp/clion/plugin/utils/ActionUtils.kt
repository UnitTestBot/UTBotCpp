package org.utbot.cpp.clion.plugin.utils

import com.intellij.openapi.actionSystem.AnActionEvent
import org.utbot.cpp.clion.plugin.settings.settings

fun isPluginEnabled(e: AnActionEvent): Boolean {
    var isEnabled = false
    val project = e.project
    if (project != null) {
        isEnabled = project.settings.storedSettings.isPluginEnabled
    }
    return isEnabled
}