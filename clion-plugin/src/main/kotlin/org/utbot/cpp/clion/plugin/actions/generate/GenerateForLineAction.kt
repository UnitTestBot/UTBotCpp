package org.utbot.cpp.clion.plugin.actions.generate

import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.CommonDataKeys
import org.utbot.cpp.clion.plugin.client.requests.test.LineRequest
import org.utbot.cpp.clion.plugin.grpc.ParamsBuilder
import org.utbot.cpp.clion.plugin.utils.activeProject

class GenerateForLineAction : BaseGenerateTestsAction() {
    override fun actionPerformed(e: AnActionEvent) {
        val filePath = e.getRequiredData(CommonDataKeys.VIRTUAL_FILE).path
        val editor = e.getRequiredData(CommonDataKeys.EDITOR)
        val lineNumber = editor.caretModel.logicalPosition.line + 1
        LineRequest(
            ParamsBuilder(e.activeProject()).buildLineRequestBuilder(lineNumber, filePath),
            e.activeProject(),
        ).execute()
    }

    override fun isDefined(e: AnActionEvent): Boolean {
        val project = e.project
        val editor = e.getData(CommonDataKeys.EDITOR)
        val file = e.getData(CommonDataKeys.VIRTUAL_FILE)

        return project != null && editor != null && file != null
    }
}
