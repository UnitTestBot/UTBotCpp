package com.huawei.utbot.cpp.ui.sourceFoldersView

import com.huawei.utbot.cpp.ui.wizard.steps.ObservableValue
import com.huawei.utbot.cpp.utils.utbotSettings
import com.intellij.ide.projectView.impl.AbstractProjectTreeStructure
import com.intellij.ide.projectView.impl.ProjectViewTree
import com.intellij.openapi.project.Project
import javax.swing.tree.DefaultTreeModel

open class UTBotProjectViewPaneForSettings(project: Project) : UTBotProjectViewPane(project) {
    private val sourceDirs: ObservableValue<Set<String>> = initObservableDirectories()

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
        return ObservableValue(myProject.utbotSettings.sourceDirs).also {
            it.addOnChangeListener {
                updateFromRoot(true)
            }
        }
    }

    fun apply() {
        myProject.utbotSettings.sourceDirs = sourceDirs.value
    }

    fun reset() {
        sourceDirs.value = myProject.utbotSettings.sourceDirs
    }

    fun isModified() = myProject.utbotSettings.sourceDirs != sourceDirs.value

    override fun createStructure() = object : AbstractProjectTreeStructure(myProject) {
        override fun getProviders() = listOf(UTBotTreeStructureProvider(isMarked = { dir -> dir.virtualFile.path in sourceDirs.value}))
    }
}
