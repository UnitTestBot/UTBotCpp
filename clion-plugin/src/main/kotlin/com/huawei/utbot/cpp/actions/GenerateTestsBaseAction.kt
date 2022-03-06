package com.huawei.utbot.cpp.actions

import com.huawei.utbot.cpp.client.Client
import com.intellij.openapi.actionSystem.AnAction
import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.components.service

abstract class GenerateTestsBaseAction : AnAction() {
    override fun update(e: AnActionEvent) {
        if (e.project?.service<Client>()?.isServerAvailable() == true) {
            updateIfServerAvailable(e)
        } else {
            e.presentation.isEnabled = false
        }
    }

    abstract fun updateIfServerAvailable(e: AnActionEvent)
}
