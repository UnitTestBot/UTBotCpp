package org.utbot.cpp.clion.plugin.actions

import com.intellij.openapi.actionSystem.AnAction
import com.intellij.openapi.actionSystem.AnActionEvent
import org.utbot.cpp.clion.plugin.utils.client

abstract class GenerateTestsBaseAction : AnAction() {
    override fun update(e: AnActionEvent) {
        if (e.client.isServerAvailable()) {
            updateIfServerAvailable(e)
        } else {
            e.presentation.isEnabled = false
        }
    }

    abstract fun updateIfServerAvailable(e: AnActionEvent)
}
