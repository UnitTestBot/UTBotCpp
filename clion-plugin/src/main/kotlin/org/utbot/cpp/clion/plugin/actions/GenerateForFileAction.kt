package org.utbot.cpp.clion.plugin.actions

import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.CommonDataKeys
import org.utbot.cpp.clion.plugin.actions.utils.getFileRequestMessage
import org.utbot.cpp.clion.plugin.client.requests.FileRequest
import org.utbot.cpp.clion.plugin.utils.isCPPorCFileName

class GenerateForFileAction : GenerateTestsBaseAction() {
    override fun actionPerformed(e: AnActionEvent) {
        FileRequest(getFileRequestMessage(e), e.project!!).execute()
    }

    // action is available only if the selected file ends in .cpp, .hpp, .c or .h
    override fun updateIfServerAvailable(e: AnActionEvent) {
        val file = e.getData(CommonDataKeys.PSI_FILE)
        e.presentation.isEnabledAndVisible = isCPPorCFileName(file?.name ?: "")
    }
}
