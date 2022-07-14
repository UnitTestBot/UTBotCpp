package com.huawei.utbot.cpp.ui.sourceFoldersView

import com.huawei.utbot.cpp.ui.UTBotIcons
import com.intellij.icons.AllIcons
import com.intellij.ide.projectView.PresentationData
import com.intellij.ide.projectView.ViewSettings
import com.intellij.ide.projectView.impl.nodes.PsiDirectoryNode
import com.intellij.openapi.project.Project
import com.intellij.psi.PsiDirectory

class UTBotDirectoryNode(
    project: Project,
    directory: PsiDirectory,
    settings: ViewSettings?,
    private val isMarked: (PsiDirectory)->Boolean
) : PsiDirectoryNode(project, directory, settings) {
    override fun update(presentation: PresentationData) {
        super.update(presentation)
        presentation.apply {
            if (isMarked(value)) {
                setIcon(UTBotIcons.SOURCE_FOLDER)
            } else {
                setIcon(AllIcons.Nodes.Folder)
            }
        }
    }
}
