package org.utbot.cpp.clion.plugin.utils

import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.CommonDataKeys
import org.utbot.cpp.clion.plugin.settings.settings


fun AnActionEvent.getLineNumberUnsafe(): Int {
    val editor = this.getRequiredData(CommonDataKeys.EDITOR)
    return editor.caretModel.logicalPosition.line + 1
}

fun AnActionEvent.getFilePathUnsafe(): String {
    return getRequiredData(CommonDataKeys.VIRTUAL_FILE).localPath.toString()
}

fun isPluginEnabled(e: AnActionEvent): Boolean {
    var isEnabled = false
    val project = e.project
    if (project != null) {
        isEnabled = project.settings.storedSettings.isPluginEnabled
    }
    return isEnabled
}