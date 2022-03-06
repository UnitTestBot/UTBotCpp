package com.huawei.utbot.cpp.actions

import com.huawei.utbot.cpp.actions.utils.getClassRequestMessage
import com.huawei.utbot.cpp.utils.client
import com.huawei.utbot.cpp.actions.utils.getContainingClass
import com.intellij.openapi.actionSystem.AnActionEvent

class GenerateForClassAction : GenerateTestsBaseAction() {
    override fun actionPerformed(e: AnActionEvent) {
        e.client.generateForClass(getClassRequestMessage(e))
    }

    override fun updateIfServerAvailable(e: AnActionEvent) {
        e.presentation.isEnabledAndVisible = (getContainingClass(e) != null)
    }
}
