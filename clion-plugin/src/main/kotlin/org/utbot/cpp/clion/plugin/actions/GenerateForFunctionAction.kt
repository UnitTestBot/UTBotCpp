package org.utbot.cpp.clion.plugin.actions

import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.CommonDataKeys
import org.utbot.cpp.clion.plugin.utils.getFunctionRequestMessage
import org.utbot.cpp.clion.plugin.client.requests.FunctionRequest

class GenerateForFunctionAction : GenerateTestsBaseAction() {
    override fun updateIfServerAvailable(e: AnActionEvent) {
        e.presentation.isEnabledAndVisible =
            e.project != null && e.getData(CommonDataKeys.EDITOR) != null && e.getData(CommonDataKeys.VIRTUAL_FILE) != null
    }

    override fun actionPerformed(e: AnActionEvent) {
        FunctionRequest(
            getFunctionRequestMessage(e),
            e.project!!
        ).execute()
    }
}
