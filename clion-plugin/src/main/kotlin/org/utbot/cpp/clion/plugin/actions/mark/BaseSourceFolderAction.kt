package org.utbot.cpp.clion.plugin.actions.mark

import com.intellij.openapi.actionSystem.AnAction
import com.intellij.openapi.actionSystem.AnActionEvent
import org.utbot.cpp.clion.plugin.ui.sourceFoldersView.ProxyProjectViewTree

abstract class BaseSourceFolderAction: AnAction() {

    override fun update(e: AnActionEvent) {
        e.presentation.isEnabledAndVisible = e.getData(ProxyProjectViewTree.UTBOT_DIRS) != null
    }
}