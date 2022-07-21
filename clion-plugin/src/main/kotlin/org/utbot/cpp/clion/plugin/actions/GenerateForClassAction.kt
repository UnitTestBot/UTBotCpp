package org.utbot.cpp.clion.plugin.actions

import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.CommonDataKeys
import org.utbot.cpp.clion.plugin.grpc.getClassRequest
import org.utbot.cpp.clion.plugin.client.requests.ClassRequest
import org.utbot.cpp.clion.plugin.utils.isCPPFileName

class GenerateForClassAction : GenerateTestsBaseAction() {
    override fun actionPerformed(e: AnActionEvent) {
        ClassRequest(
            getClassRequest(e),
            e.project!!,
        ).execute()
    }

    override fun updateIfServerAvailable(e: AnActionEvent) {
        e.presentation.isEnabledAndVisible = e.project != null &&
                e.getData(CommonDataKeys.EDITOR) != null &&
                e.getData(CommonDataKeys.VIRTUAL_FILE) != null &&
                isCPPFileName(e.getRequiredData(CommonDataKeys.VIRTUAL_FILE).name)
    }
}
