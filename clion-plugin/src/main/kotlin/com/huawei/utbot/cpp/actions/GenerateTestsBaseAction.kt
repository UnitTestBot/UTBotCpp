package com.huawei.utbot.cpp.actions

import com.huawei.utbot.cpp.utils.client
import com.intellij.openapi.actionSystem.AnAction
import com.intellij.openapi.actionSystem.AnActionEvent

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
