package org.utbot.cpp.clion.plugin.actions

import com.intellij.openapi.actionSystem.AnAction
import com.intellij.openapi.actionSystem.AnActionEvent
import org.utbot.cpp.clion.plugin.ui.sourceFoldersView.ProxyProjectViewTree

class MarkSourceFolderAction: AnAction() {
    override fun actionPerformed(e: AnActionEvent) {
        val update = e.getData(ProxyProjectViewTree.UTBOT_DIRS)!!
        update.markAsSource()
    }

    override fun update(e: AnActionEvent) {
        e.presentation.isEnabledAndVisible = e.getData(ProxyProjectViewTree.UTBOT_DIRS) != null
    }
}
