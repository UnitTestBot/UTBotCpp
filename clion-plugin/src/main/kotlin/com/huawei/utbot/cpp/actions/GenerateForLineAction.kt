package com.huawei.utbot.cpp.actions

import com.huawei.utbot.cpp.actions.utils.getLineRequestMessage
import com.huawei.utbot.cpp.client.Requests.LineRequest
import com.huawei.utbot.cpp.utils.execute
import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.CommonDataKeys

class GenerateForLineAction : GenerateTestsBaseAction() {
    override fun updateIfServerAvailable(e: AnActionEvent) {
        val project = e.project
        val editor = e.getData(CommonDataKeys.EDITOR)
        val file = e.getData(CommonDataKeys.VIRTUAL_FILE)
        e.presentation.isEnabledAndVisible = (project != null) && (editor != null) && (file != null)
    }

    override fun actionPerformed(e: AnActionEvent) {
        LineRequest(
            getLineRequestMessage(e),
            e.project!!
        ).execute(e)
    }
}
