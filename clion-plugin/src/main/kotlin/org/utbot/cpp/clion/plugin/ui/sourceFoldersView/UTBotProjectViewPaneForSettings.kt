package org.utbot.cpp.clion.plugin.ui.sourceFoldersView

import com.intellij.ide.projectView.impl.AbstractProjectTreeStructure
import com.intellij.ide.projectView.impl.ProjectViewTree
import com.intellij.openapi.project.Project
import org.utbot.cpp.clion.plugin.settings.UTBotProjectStoredSettings
import org.utbot.cpp.clion.plugin.settings.settings
import org.utbot.cpp.clion.plugin.ui.wizard.steps.ObservableValue
import org.utbot.cpp.clion.plugin.utils.localPath
import javax.swing.tree.DefaultTreeModel

open class UTBotProjectViewPaneForSettings(project: Project) : UTBotProjectViewPane(project) {
    private val sourceDirs: ObservableValue<Set<String>> = initObservableDirectories()

    private val settings: UTBotProjectStoredSettings.State
        get() = myProject.settings.storedSettings

    override fun createTree(treeModel: DefaultTreeModel): ProjectViewTree {
        return object : ProxyProjectViewTree(treeModel, myProject, this@UTBotProjectViewPaneForSettings) {
            override fun createUpdater(): BaseUpdater = object : BaseUpdater(selectedDirectories.toList()) {
                override fun getCurrentMarkedDirs(): Set<String> = sourceDirs.value
                override fun setCurrentMarkedDirs(value: Set<String>) {
                    sourceDirs.value = value
                }
            }
        }
    }

    private fun initObservableDirectories(): ObservableValue<Set<String>> {
        return ObservableValue(settings.sourceDirs).also {
            it.addOnChangeListener {
                updateFromRoot(true)
            }
        }
    }

    fun apply() {
        settings.sourceDirs = sourceDirs.value
    }

    fun reset() {
        sourceDirs.value = settings.sourceDirs
    }

    fun isModified() = settings.sourceDirs != sourceDirs.value

    override fun createStructure() = object : AbstractProjectTreeStructure(myProject) {
        override fun getProviders() = listOf(UTBotTreeStructureProvider(isMarked = { dir -> dir.virtualFile.localPath.toString() in sourceDirs.value}))
    }
}
