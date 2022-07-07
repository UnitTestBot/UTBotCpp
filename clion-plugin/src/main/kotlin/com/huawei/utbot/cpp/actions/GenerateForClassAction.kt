package com.huawei.utbot.cpp.actions

import com.huawei.utbot.cpp.actions.utils.getClassRequestMessage
import com.huawei.utbot.cpp.client.requests.ClassRequest
import com.huawei.utbot.cpp.utils.execute
import com.huawei.utbot.cpp.utils.isCPPFileName
import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.CommonDataKeys

class GenerateForClassAction : GenerateTestsBaseAction() {
    override fun actionPerformed(e: AnActionEvent) {
        ClassRequest(
            getClassRequestMessage(e),
            e.project!!,
        ).execute(e)
    }

    override fun updateIfServerAvailable(e: AnActionEvent) {
        e.presentation.isEnabledAndVisible = e.project != null &&
                e.getData(CommonDataKeys.EDITOR) != null &&
                e.getData(CommonDataKeys.VIRTUAL_FILE) != null &&
                isCPPFileName(e.getRequiredData(CommonDataKeys.VIRTUAL_FILE).name)
    }
}
