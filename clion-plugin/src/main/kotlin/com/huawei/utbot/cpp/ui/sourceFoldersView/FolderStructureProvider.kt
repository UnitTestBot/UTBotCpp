package com.huawei.utbot.cpp.ui.sourceFoldersView

import com.intellij.ide.projectView.TreeStructureProvider
import com.intellij.ide.projectView.ViewSettings
import com.intellij.ide.projectView.impl.nodes.PsiDirectoryNode
import com.intellij.ide.util.treeView.AbstractTreeNode

class FolderStructureProvider : TreeStructureProvider {
    override fun modify(
        parent: AbstractTreeNode<*>,
        children: MutableCollection<AbstractTreeNode<*>>,
        settings: ViewSettings?
    ): MutableCollection<AbstractTreeNode<*>> {
        val nodes = mutableListOf<AbstractTreeNode<*>>()

        for (child in children) {
            if (child is PsiDirectoryNode) {
                nodes.add(UTBotDirectoryNode(child.project, child.value, settings))
            } else {
                nodes.add(child)
            }
        }
        return nodes
    }
}
