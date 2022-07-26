package org.utbot.cpp.clion.plugin.ui.sourceFoldersView

import com.intellij.ide.projectView.TreeStructureProvider
import com.intellij.ide.projectView.ViewSettings
import com.intellij.ide.projectView.impl.nodes.PsiDirectoryNode
import com.intellij.ide.util.treeView.AbstractTreeNode
import com.intellij.psi.PsiDirectory

class UTBotTreeStructureProvider(
    private val isMarked: (PsiDirectory)->Boolean
) : TreeStructureProvider {
    override fun modify(
        parent: AbstractTreeNode<*>,
        children: MutableCollection<AbstractTreeNode<*>>,
        settings: ViewSettings?
    ): MutableCollection<AbstractTreeNode<*>> {
        return children.map {
            if (it is PsiDirectoryNode)
                UTBotDirectoryNode(it.project, it.value, it.settings, isMarked)
            else
                it
        }.toMutableList()
    }
}
