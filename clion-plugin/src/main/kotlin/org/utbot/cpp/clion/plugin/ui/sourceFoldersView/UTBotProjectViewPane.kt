package org.utbot.cpp.clion.plugin.ui.sourceFoldersView

import com.intellij.ide.SelectInTarget
import com.intellij.ide.impl.ProjectPaneSelectInTarget
import com.intellij.ide.projectView.impl.AbstractProjectTreeStructure
import com.intellij.ide.projectView.impl.ProjectViewPane
import com.intellij.ide.projectView.impl.ProjectViewTree
import com.intellij.openapi.project.Project
import com.intellij.psi.PsiFileSystemItem
import org.utbot.cpp.clion.plugin.listeners.SourceFoldersListener
import org.utbot.cpp.clion.plugin.settings.settings
import org.utbot.cpp.clion.plugin.utils.localPath
import javax.swing.tree.DefaultTreeModel
import org.utbot.cpp.clion.plugin.utils.projectLifetimeDisposable

open class UTBotProjectViewPane(project: Project) : ProjectViewPane(project) {
    override fun enableDnD() = Unit
    override fun getId(): String = "UTBotProjectPane"
    override fun getTitle(): String = "UTBot: Source Directories"
    override fun getPresentableSubIdName(subId: String): String = "UTBotSourceDirectoriesPane"

    init {
        // this connection is needed during project lifetime, so pass project service ClientManager as parent disposable
        project.messageBus.connect(project.projectLifetimeDisposable).subscribe(SourceFoldersListener.TOPIC, SourceFoldersListener {
            // when sourceDirs are updated in model, update view
            // it will eventually call node.update, see UTBotNode
            updateFromRoot(true)
        })
    }

    override fun getWeight(): Int = 5

    // required by IJ api, but not used. Otherwise, there are exceptions
    override fun createSelectInTarget(): SelectInTarget {
        return object : ProjectPaneSelectInTarget(myProject) {
            override fun toString(): String = "UTBot: Directories"
            override fun getMinorViewId() = this@UTBotProjectViewPane.id
            override fun canSelect(file: PsiFileSystemItem?): Boolean = false
        }
    }

    override fun createTree(treeModel: DefaultTreeModel): ProjectViewTree {
        return ProxyProjectViewTree(treeModel, myProject, this)
    }

    override fun createStructure() = object : AbstractProjectTreeStructure(myProject) {
        // replace directory nodes with our UTBotNodes, which check source dirs during node.update
        override fun getProviders() = listOf(
            UTBotTreeStructureProvider(isMarked = { dir -> dir.virtualFile.localPath.toString() in myProject.settings.storedSettings.sourceDirs })
        )
    }
}
