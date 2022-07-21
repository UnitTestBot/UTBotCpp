package org.utbot.cpp.clion.plugin.actions

import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.CommonDataKeys
import org.utbot.cpp.clion.plugin.grpc.getFolderRequest
import org.utbot.cpp.clion.plugin.client.requests.FolderRequest

class GenerateForFolderAction : GenerateTestsBaseAction() {
    override fun actionPerformed(e: AnActionEvent) {
        FolderRequest(
            getFolderRequest(e),
            e.project!!
        ).execute()
    }

    override fun updateIfServerAvailable(e: AnActionEvent) {
        if (e.project == null) {
            e.presentation.isEnabledAndVisible = false
            return
        }
        e.presentation.isEnabledAndVisible = e.getData(CommonDataKeys.VIRTUAL_FILE)?.isDirectory ?: false
    }
}
