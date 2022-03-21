package com.huawei.utbot.cpp.actions

import com.huawei.utbot.cpp.actions.utils.getAssertionRequestMessage
import com.huawei.utbot.cpp.utils.client
import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.CommonDataKeys

class GenerateForAssertionAction : GenerateTestsBaseAction() {
    override fun actionPerformed(e: AnActionEvent) {
        e.client.generateForAssertion(getAssertionRequestMessage(e))
    }

    override fun updateIfServerAvailable(e: AnActionEvent) {
        val project = e.project
        val editor = e.getData(CommonDataKeys.EDITOR)
        val file = e.getData(CommonDataKeys.VIRTUAL_FILE)
        e.presentation.isEnabledAndVisible = (project != null) && (editor != null) && (file != null)
    }
}