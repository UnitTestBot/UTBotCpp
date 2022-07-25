package org.utbot.cpp.clion.plugin.actions

import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.CommonDataKeys
import org.utbot.cpp.clion.plugin.grpc.getLineGrpcRequest
import org.utbot.cpp.clion.plugin.client.requests.LineRequest

class GenerateForLineAction : GenerateTestsBaseAction() {
    override fun updateIfServerAvailable(e: AnActionEvent) {
        val project = e.project
        val editor = e.getData(CommonDataKeys.EDITOR)
        val file = e.getData(CommonDataKeys.VIRTUAL_FILE)
        e.presentation.isEnabledAndVisible = (project != null) && (editor != null) && (file != null)
    }

    override fun actionPerformed(e: AnActionEvent) {
        LineRequest(
            getLineGrpcRequest(e),
            e.project!!
        ).execute()
    }
}
