package org.utbot.cpp.clion.plugin.actions.generate

import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.CommonDataKeys
import org.utbot.cpp.clion.plugin.client.requests.test.FileRequest
import org.utbot.cpp.clion.plugin.grpc.ParamsBuilder
import org.utbot.cpp.clion.plugin.utils.activeProject
import org.utbot.cpp.clion.plugin.utils.getFilePathUnsafe
import org.utbot.cpp.clion.plugin.utils.isCPPorCFileName

class GenerateForFileAction : BaseGenerateTestsAction() {
    override fun actionPerformed(e: AnActionEvent) =
        FileRequest(
            ParamsBuilder(e.activeProject()).buildFileRequestParams(e.getFilePathUnsafe()),
            e.activeProject(),
        ).execute()

    // action is available only if the selected file ends in .cpp, .hpp, .c or .h
    override fun isDefined(e: AnActionEvent): Boolean {
        val file = e.getData(CommonDataKeys.PSI_FILE)
        return isCPPorCFileName(file?.name ?: "")
    }
}
