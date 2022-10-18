package org.utbot.cpp.clion.plugin.actions.generate

import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.CommonDataKeys
import org.utbot.cpp.clion.plugin.client.requests.test.FunctionRequest
import org.utbot.cpp.clion.plugin.grpc.getFunctionGrpcRequest
import org.utbot.cpp.clion.plugin.utils.activeProject

class GenerateForFunctionAction : BaseGenerateTestsAction() {

    override fun actionPerformed(e: AnActionEvent) =
        FunctionRequest(
            getFunctionGrpcRequest(e),
            e.activeProject()
        ).execute()

    override fun isDefined(e: AnActionEvent): Boolean {
        val project = e.project
        val editor = e.getData(CommonDataKeys.EDITOR)
        val file = e.getData(CommonDataKeys.VIRTUAL_FILE)

        return project != null && editor != null && file != null
    }
}
