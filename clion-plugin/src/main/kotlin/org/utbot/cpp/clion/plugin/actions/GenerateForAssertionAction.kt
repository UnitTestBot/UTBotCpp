package org.utbot.cpp.clion.plugin.actions

import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.CommonDataKeys
import org.utbot.cpp.clion.plugin.utils.getAssertionRequestMessage
import org.utbot.cpp.clion.plugin.client.requests.AssertionRequest

class GenerateForAssertionAction : GenerateTestsBaseAction() {
    override fun actionPerformed(e: AnActionEvent) {
        AssertionRequest(
            getAssertionRequestMessage(e),
            e.project!!,
        ).execute()
    }

    override fun updateIfServerAvailable(e: AnActionEvent) {
        val project = e.project
        val editor = e.getData(CommonDataKeys.EDITOR)
        val file = e.getData(CommonDataKeys.VIRTUAL_FILE)
        e.presentation.isEnabledAndVisible = (project != null) && (editor != null) && (file != null)
    }
}
