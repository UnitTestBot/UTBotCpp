package com.huawei.utbot.cpp.actions

import com.huawei.utbot.cpp.utils.utbotSettings
import com.huawei.utbot.cpp.utils.visitAllDirectories
import com.intellij.ide.projectView.ProjectView
import com.intellij.openapi.actionSystem.AnAction
import com.intellij.openapi.actionSystem.AnActionEvent

class MarkSourceFolderAction: AnAction() {
    override fun actionPerformed(e: AnActionEvent) {
        val project = e.project!!
        val newSourceFolders: MutableSet<String> = project.utbotSettings.sourcePaths.toMutableSet()
        ProjectView.getInstance(project).currentProjectViewPane.selectedDirectories.forEach { dir ->
            newSourceFolders.add(dir.virtualFile.path)
            dir.virtualFile.toNioPath().visitAllDirectories {
                newSourceFolders.add(it.toString())
            }
        }
        project.utbotSettings.sourcePaths = newSourceFolders
    }
}
