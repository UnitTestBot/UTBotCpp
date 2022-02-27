package actions

import actions.utils.client
import com.github.vol0n.utbotcppclion.utils.isCPPorCFileName
import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.CommonDataKeys

class GenerateForFileAction : GenerateTestsBaseAction() {
    override fun actionPerformed(e: AnActionEvent) {
        e.client.generateForFile(getFileRequestMessage(e))
    }

    // action is available only if the selected file ends in .cpp, .hpp, .c or .h
    override fun updateIfServerAvailable(e: AnActionEvent) {
        val file = e.getData(CommonDataKeys.PSI_FILE)
        e.presentation.isEnabledAndVisible = isCPPorCFileName(file?.name ?: "")
    }
}
