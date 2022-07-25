package org.utbot.cpp.clion.plugin.actions.generate

import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.CommonDataKeys
import org.utbot.cpp.clion.plugin.client.requests.SnippetRequest
import org.utbot.cpp.clion.plugin.grpc.getSnippetGrpcRequest
import org.utbot.cpp.clion.plugin.utils.activeProject

class GenerateForSnippetAction : BaseGenerateTestsAction() {
    override fun actionPerformed(e: AnActionEvent) {
        SnippetRequest(
            getSnippetGrpcRequest(e),
            e.activeProject(),
        ).execute()
    }

    override fun isDefined(e: AnActionEvent): Boolean {
        val file = e.getData(CommonDataKeys.VIRTUAL_FILE)
        return file != null
    }
}
