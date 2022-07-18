package org.utbot.cpp.clion.plugin.actions

import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.CommonDataKeys
import org.utbot.cpp.clion.plugin.actions.utils.getSnippetRequestMessage
import org.utbot.cpp.clion.plugin.client.requests.SnippetRequest

class GenerateForSnippetAction : GenerateTestsBaseAction() {
    override fun actionPerformed(e: AnActionEvent) {
        SnippetRequest(
            getSnippetRequestMessage(e),
            e.project!!
        ).execute()
    }

    override fun updateIfServerAvailable(e: AnActionEvent) {
        val file = e.getData(CommonDataKeys.VIRTUAL_FILE)
        e.presentation.isEnabledAndVisible = file != null
    }
}
