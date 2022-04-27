package com.huawei.utbot.cpp.actions

import com.huawei.utbot.cpp.actions.utils.getFunctionRequestMessage
import com.huawei.utbot.cpp.actions.utils.getContainingFunction
import com.huawei.utbot.cpp.client.Requests.FunctionRequest
import com.huawei.utbot.cpp.utils.execute
import com.intellij.openapi.actionSystem.AnActionEvent

class GenerateForFunctionAction : GenerateTestsBaseAction() {
    override fun updateIfServerAvailable(e: AnActionEvent) {
        e.presentation.isEnabledAndVisible = (getContainingFunction(e) != null)
    }

    override fun actionPerformed(e: AnActionEvent) {
        FunctionRequest(
            getFunctionRequestMessage(e),
            e.project!!
        ).execute(e)
    }
}
