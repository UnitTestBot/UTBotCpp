package org.utbot.cpp.clion.plugin.actions.generate

import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.CommonDataKeys
import org.utbot.cpp.clion.plugin.client.requests.test.ClassRequest
import org.utbot.cpp.clion.plugin.grpc.ParamsBuilder
import org.utbot.cpp.clion.plugin.utils.activeProject
import org.utbot.cpp.clion.plugin.utils.getFilePathUnsafe
import org.utbot.cpp.clion.plugin.utils.getLineNumberUnsafe
import org.utbot.cpp.clion.plugin.utils.isCPPFileName

class GenerateForClassAction : BaseGenerateTestsAction() {
    override fun actionPerformed(e: AnActionEvent) =
        ClassRequest(
            ParamsBuilder(e.activeProject()).buildClassRequestParams(e.getFilePathUnsafe(), e.getLineNumberUnsafe()),
            e.activeProject(),
        ).execute()

    override fun isDefined(e: AnActionEvent): Boolean {
        val project = e.project
        val editor = e.getData(CommonDataKeys.EDITOR)
        val file = e.getData(CommonDataKeys.VIRTUAL_FILE)

        return project != null && editor != null && file != null && isCPPFileName(file.name)
    }
}
