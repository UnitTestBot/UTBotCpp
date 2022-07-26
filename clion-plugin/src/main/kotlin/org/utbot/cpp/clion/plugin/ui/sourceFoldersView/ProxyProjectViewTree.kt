package org.utbot.cpp.clion.plugin.ui.sourceFoldersView

import com.intellij.ide.projectView.impl.ProjectViewTree
import com.intellij.openapi.actionSystem.DataKey
import com.intellij.openapi.actionSystem.DataProvider
import com.intellij.openapi.project.Project
import javax.swing.tree.DefaultTreeModel
import org.utbot.cpp.clion.plugin.utils.utbotSettings
import java.awt.event.MouseAdapter
import java.awt.event.MouseEvent

open class ProxyProjectViewTree(
    treeModel: DefaultTreeModel,
    val project: Project,
    private val myPane: UTBotProjectViewPane,
) : ProjectViewTree(treeModel), DataProvider {

    init {
        setup()
    }

    fun setup() {
        // disable node expansion on double click
        setToggleClickCount(0)
        // change marked/unmarked status on double click
        addMouseListener(object : MouseAdapter() {
            override fun mousePressed(e: MouseEvent?) {
                if (e?.clickCount == 2) {
                    createUpdater().toggle()
                }
            }
        })
    }

    protected open fun createUpdater() = object : BaseUpdater(myPane.selectedDirectories.toList()) {
        override fun getCurrentMarkedDirs(): Set<String> = project.utbotSettings.sourceDirs
        override fun setCurrentMarkedDirs(value: Set<String>) {
            project.utbotSettings.sourceDirs = value.toMutableSet()
        }
    }

    override fun getData(dataId: String): Any? {
        if (dataId == key) {
            return createUpdater()
        }
        return null
    }

    companion object {
        const val key = "UTBotDirectories"
        val UTBOT_DIRS = DataKey.create<DirectoriesStatusUpdater>(key)
    }
}
