package org.utbot.cpp.clion.plugin.actions.mark

import com.intellij.openapi.actionSystem.AnActionEvent
import org.utbot.cpp.clion.plugin.ui.sourceFoldersView.ProxyProjectViewTree

class UnmarkSourceFolderAction: BaseSourceFolderAction() {

    override fun actionPerformed(e: AnActionEvent) {
        val directoryUpdater = e.getData(ProxyProjectViewTree.UTBOT_DIRS)
            ?: error("UnmarkSourceFolderAction should be disabled for event $e")

        directoryUpdater.unmarkAsSource()
    }
}
