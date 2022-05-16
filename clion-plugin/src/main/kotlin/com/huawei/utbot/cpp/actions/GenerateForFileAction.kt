package com.huawei.utbot.cpp.actions

import com.huawei.utbot.cpp.actions.utils.getFileRequestMessage
import com.huawei.utbot.cpp.client.requests.FileRequest
import com.huawei.utbot.cpp.utils.execute
import com.huawei.utbot.cpp.utils.isCPPorCFileName
import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.CommonDataKeys

class GenerateForFileAction : GenerateTestsBaseAction() {
    override fun actionPerformed(e: AnActionEvent) {
        FileRequest(getFileRequestMessage(e), e.project!!).execute(e)
    }

    // action is available only if the selected file ends in .cpp, .hpp, .c or .h
    override fun updateIfServerAvailable(e: AnActionEvent) {
        val file = e.getData(CommonDataKeys.PSI_FILE)
        e.presentation.isEnabledAndVisible = isCPPorCFileName(file?.name ?: "")
    }
}
