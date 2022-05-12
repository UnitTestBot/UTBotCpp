package com.huawei.utbot.cpp.ui.sourceFoldersView

import com.huawei.utbot.cpp.ui.UTBotIcons
import com.huawei.utbot.cpp.utils.utbotSettings
import com.intellij.icons.AllIcons
import com.intellij.ide.projectView.PresentationData
import com.intellij.ide.projectView.ProjectViewNode
import com.intellij.ide.projectView.ProjectViewNodeDecorator
import com.intellij.ide.projectView.impl.nodes.PsiDirectoryNode
import com.intellij.packageDependencies.ui.PackageDependenciesNode
import com.intellij.ui.ColoredTreeCellRenderer

class FolderDecorator: ProjectViewNodeDecorator {
    override fun decorate(node: ProjectViewNode<*>, data: PresentationData) {
        if (node is PsiDirectoryNode) {
            val file = node.virtualFile
            val sourceFolders = node.project.utbotSettings.sourcePaths
            if (file?.path in sourceFolders) {
                data.setIcon(UTBotIcons.SOURCE_FOLDER)
            } else {
                data.setIcon(AllIcons.Nodes.Folder)
            }
        }
    }

    override fun decorate(node: PackageDependenciesNode?, cellRenderer: ColoredTreeCellRenderer?) {}
}
