package actions

import actions.utils.client
import actions.utils.getContainingClass
import com.intellij.openapi.actionSystem.AnActionEvent

class GenerateForClassAction : GenerateTestsBaseAction() {
    override fun actionPerformed(e: AnActionEvent) {
        e.client.generateForClass(getClassRequestMessage(e))
    }

    override fun updateIfServerAvailable(e: AnActionEvent) {
        e.presentation.isEnabledAndVisible = (getContainingClass(e) != null)
    }
}
