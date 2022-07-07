package com.huawei.utbot.cpp.actions

import com.huawei.utbot.cpp.actions.utils.getFunctionRequestMessage
import com.huawei.utbot.cpp.client.requests.FunctionRequest
import com.huawei.utbot.cpp.utils.execute
import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.CommonDataKeys

class GenerateForFunctionAction : GenerateTestsBaseAction() {
    override fun updateIfServerAvailable(e: AnActionEvent) {
        e.presentation.isEnabledAndVisible =
            e.project != null && e.getData(CommonDataKeys.EDITOR) != null && e.getData(CommonDataKeys.VIRTUAL_FILE) != null
    }

    override fun actionPerformed(e: AnActionEvent) {
        FunctionRequest(
            getFunctionRequestMessage(e),
            e.project!!
        ).execute(e)
    }
}
