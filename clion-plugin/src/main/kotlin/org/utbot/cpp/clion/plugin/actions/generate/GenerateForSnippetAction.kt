package org.utbot.cpp.clion.plugin.actions.generate

import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.CommonDataKeys
import org.utbot.cpp.clion.plugin.client.requests.test.SnippetRequest
import org.utbot.cpp.clion.plugin.grpc.GrpcRequestBuilderFactory
import org.utbot.cpp.clion.plugin.utils.activeProject
import org.utbot.cpp.clion.plugin.utils.getFilePathUnsafe

class GenerateForSnippetAction : BaseGenerateTestsAction() {
    override fun actionPerformed(e: AnActionEvent) {
        SnippetRequest(
            GrpcRequestBuilderFactory(e.activeProject()).createSnippetRequestBuilder(e.getFilePathUnsafe()),
            e.activeProject(),
        ).execute()
    }

    override fun isDefined(e: AnActionEvent): Boolean {
        val file = e.getData(CommonDataKeys.VIRTUAL_FILE)
        return file != null
    }
}
