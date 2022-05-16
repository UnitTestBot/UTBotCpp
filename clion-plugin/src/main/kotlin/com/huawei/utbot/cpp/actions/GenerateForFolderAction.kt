package com.huawei.utbot.cpp.actions

import com.huawei.utbot.cpp.actions.utils.getFolderRequestMessage
import com.huawei.utbot.cpp.client.requests.FolderRequest
import com.huawei.utbot.cpp.utils.execute
import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.CommonDataKeys

class GenerateForFolderAction : GenerateTestsBaseAction() {
    override fun actionPerformed(e: AnActionEvent) {
        FolderRequest(
            getFolderRequestMessage(e),
            e.project!!
        ).execute(e)
    }

    override fun updateIfServerAvailable(e: AnActionEvent) {
        if (e.project?.basePath == null) {
            e.presentation.isEnabledAndVisible = false
            return
        }
        e.presentation.isEnabledAndVisible = e.getData(CommonDataKeys.VIRTUAL_FILE)?.isDirectory ?: false
    }
}
