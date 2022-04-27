package com.huawei.utbot.cpp.actions

import com.huawei.utbot.cpp.actions.utils.getClassRequestMessage
import com.huawei.utbot.cpp.actions.utils.getContainingClass
import com.huawei.utbot.cpp.client.Requests.ClassRequest
import com.huawei.utbot.cpp.utils.execute
import com.intellij.openapi.actionSystem.AnActionEvent

class GenerateForClassAction : GenerateTestsBaseAction() {
    override fun actionPerformed(e: AnActionEvent) {
        ClassRequest(
            getClassRequestMessage(e),
            e.project!!,
        ).execute(e)
    }

    override fun updateIfServerAvailable(e: AnActionEvent) {
        e.presentation.isEnabledAndVisible = (getContainingClass(e) != null)
    }
}
