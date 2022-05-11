package com.huawei.utbot.cpp.ui.sourceFoldersView

import com.huawei.utbot.cpp.messaging.SourceFoldersListener
import com.huawei.utbot.cpp.services.UTBotSettings
import com.huawei.utbot.cpp.ui.UTBotIcons
import com.huawei.utbot.cpp.utils.visitAllFiles
import com.intellij.icons.AllIcons
import com.intellij.ide.projectView.PresentationData
import com.intellij.ide.projectView.ViewSettings
import com.intellij.ide.projectView.impl.ProjectAbstractTreeStructureBase
import com.intellij.ide.projectView.impl.nodes.PsiDirectoryNode
import com.intellij.ide.projectView.impl.nodes.PsiFileNode
import com.intellij.openapi.components.service
import com.intellij.openapi.project.Project
import com.intellij.psi.PsiDirectory
import com.intellij.psi.PsiFile
import com.intellij.util.PlatformIcons
import com.intellij.util.io.isDirectory
import java.awt.Color
import java.nio.file.FileVisitResult
import java.nio.file.Files
import java.nio.file.Path
import java.nio.file.SimpleFileVisitor
import java.nio.file.attribute.BasicFileAttributes

class UTBotDirectoryNode(project: Project, value: PsiDirectory, viewSettings: ViewSettings?): PsiDirectoryNode(project, value, viewSettings) {
    init {
        project.messageBus.connect().subscribe(SourceFoldersListener.TOPIC, object : SourceFoldersListener {
            override fun sourceFoldersChanged(newSourceFolders: Set<String>) {
                updatePresentation()
            }
        })
    }

    private fun updatePresentation() {
        if (isMarked()) {
            presentation.setIcon(UTBotIcons.SOURCE_FOLDER)
        } else {
            presentation.setIcon(AllIcons.Nodes.Folder)
        }
    }

    private fun isMarked() = value.virtualFile.path in myProject.service<UTBotSettings>().sourcePaths
}
