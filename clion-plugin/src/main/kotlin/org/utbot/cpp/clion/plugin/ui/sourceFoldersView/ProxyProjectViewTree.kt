package org.utbot.cpp.clion.plugin.ui.sourceFoldersView

import com.intellij.ide.projectView.impl.ProjectViewTree
import com.intellij.openapi.actionSystem.DataKey
import com.intellij.openapi.actionSystem.DataProvider
import com.intellij.openapi.project.Project
import org.utbot.cpp.clion.plugin.settings.settings
import java.awt.event.MouseAdapter
import java.awt.event.MouseEvent
import javax.swing.tree.DefaultTreeModel
import org.utbot.cpp.clion.plugin.listeners.SourceFoldersListener

open class ProxyProjectViewTree(
    treeModel: DefaultTreeModel,
    val project: Project,
    private val myPane: UTBotProjectViewPane,
) : ProjectViewTree(treeModel), DataProvider {

    protected open fun createUpdater() = object : BaseUpdater(myPane.selectedDirectories.toList()) {
        override fun getCurrentMarkedDirs(): Set<String> = project.settings.storedSettings.sourceDirs
        override fun setCurrentMarkedDirs(value: Set<String>) {
            project.settings.storedSettings.sourceDirs = value.toMutableSet()
            project.messageBus.syncPublisher(SourceFoldersListener.TOPIC).sourceFoldersChanged(value)
        }
    }

    override fun getData(dataId: String): Any? {
        if (dataId != UTBOT_DIRS_KEY) {
            return null
        }
        return createUpdater()
    }

    companion object {
        const val UTBOT_DIRS_KEY = "UTBotDirectories"
        val UTBOT_DIRS = DataKey.create<DirectoriesStatusUpdater>(UTBOT_DIRS_KEY)
    }
}
