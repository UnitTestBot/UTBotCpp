package org.utbot.cpp.clion.plugin.ui.sourceFoldersView

import com.intellij.icons.AllIcons
import com.intellij.ide.projectView.PresentationData
import com.intellij.ide.projectView.ViewSettings
import com.intellij.ide.projectView.impl.nodes.PsiDirectoryNode
import com.intellij.openapi.project.Project
import com.intellij.psi.PsiDirectory
import org.utbot.cpp.clion.plugin.ui.UTBotIcons

class UTBotDirectoryNode(
    project: Project,
    directory: PsiDirectory,
    settings: ViewSettings?,
    private val isMarked: (PsiDirectory)->Boolean
) : PsiDirectoryNode(project, directory, settings) {

    override fun update(presentation: PresentationData) {
        super.update(presentation)
        presentation.apply {
            val icon = if (isMarked(value)) UTBotIcons.SOURCE_FOLDER else AllIcons.Nodes.Folder
            setIcon(icon)
        }
    }
}
