package org.utbot.cpp.clion.plugin.actions.generate

import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.CommonDataKeys
import org.utbot.cpp.clion.plugin.client.requests.test.FolderRequest
import org.utbot.cpp.clion.plugin.grpc.IllegalActionEventException
import org.utbot.cpp.clion.plugin.grpc.ParamsBuilder
import org.utbot.cpp.clion.plugin.utils.activeProject
import org.utbot.cpp.clion.plugin.utils.getFilePathUnsafe

class GenerateForFolderAction : BaseGenerateTestsAction() {
    override fun actionPerformed(e: AnActionEvent) =
        try {
            FolderRequest(
                ParamsBuilder(e.activeProject()).buildFolderRequestParams(e.getFilePathUnsafe()),
                e.activeProject()
            ).executeUsingCurrentClient()
        } catch (exception: IllegalActionEventException) {
            // should never happen, all keys should be checked in update
            exception.notifyUser()
        }

    override fun isDefined(e: AnActionEvent): Boolean {
        val project = e.project
        val file = e.getData(CommonDataKeys.VIRTUAL_FILE)

        return project != null && file?.isDirectory == true
    }
}
