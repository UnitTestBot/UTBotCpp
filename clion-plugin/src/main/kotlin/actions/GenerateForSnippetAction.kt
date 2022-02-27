package actions

import actions.utils.client
import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.CommonDataKeys

class GenerateForSnippetAction : GenerateTestsBaseAction() {
    override fun actionPerformed(e: AnActionEvent) {
        e.client.generateForSnippet(getSnippetRequestMessage(e))
    }

    override fun updateIfServerAvailable(e: AnActionEvent) {
        val projectPath = e.project?.basePath
        val file = e.getData(CommonDataKeys.VIRTUAL_FILE)
        e.presentation.isEnabledAndVisible = (projectPath != null && file != null)
    }
}
