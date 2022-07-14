package com.huawei.utbot.cpp.actions

import com.huawei.utbot.cpp.ui.sourceFoldersView.ProxyProjectViewTree
import com.intellij.openapi.actionSystem.AnAction
import com.intellij.openapi.actionSystem.AnActionEvent

class UnmarkSourceFolderAction: AnAction() {
    override fun actionPerformed(e: AnActionEvent) {
        val update = e.getData(ProxyProjectViewTree.UTBOT_DIRS)!!
        update.unmarkAsSource()
    }

    override fun update(e: AnActionEvent) {
        e.presentation.isEnabledAndVisible = e.getData(ProxyProjectViewTree.UTBOT_DIRS) != null
    }
}
