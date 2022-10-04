package org.utbot.cpp.clion.plugin.actions.generate

import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.CommonDataKeys
import org.utbot.cpp.clion.plugin.client.requests.test.FolderRequest
import org.utbot.cpp.clion.plugin.grpc.ParamsBuilder
import org.utbot.cpp.clion.plugin.utils.activeProject
import org.utbot.cpp.clion.plugin.utils.getFilePathUnsafe

class GenerateForFolderAction : BaseGenerateTestsAction() {
    override fun actionPerformed(e: AnActionEvent) =
            FolderRequest(
                ParamsBuilder(e.activeProject()).buildFolderRequestParams(e.getFilePathUnsafe()),
                e.activeProject()
            ).execute()

    override fun isDefined(e: AnActionEvent): Boolean {
        val project = e.project
        val file = e.getData(CommonDataKeys.VIRTUAL_FILE)

        return project != null && file?.isDirectory == true
    }
}
