package com.huawei.utbot.cpp.actions

import com.huawei.utbot.cpp.actions.utils.getFunctionRequestMessage
import com.huawei.utbot.cpp.utils.client
import com.huawei.utbot.cpp.actions.utils.getContainingFunction
import com.intellij.openapi.actionSystem.AnActionEvent

class GenerateForFunctionAction : GenerateTestsBaseAction() {
    override fun updateIfServerAvailable(e: AnActionEvent) {
        val containingFun = getContainingFunction(e)
        e.presentation.isEnabledAndVisible = (containingFun != null)
    }

    override fun actionPerformed(e: AnActionEvent) {
        e.client.generateForFunction(getFunctionRequestMessage(e))
    }
}
