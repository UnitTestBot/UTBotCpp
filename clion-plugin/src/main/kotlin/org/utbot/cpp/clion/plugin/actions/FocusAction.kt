package org.utbot.cpp.clion.plugin.actions

import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.vfs.LocalFileSystem
import com.intellij.psi.PsiManager
import org.utbot.cpp.clion.plugin.utils.activeProject
import java.nio.file.Path

class FocusAction(val path: Path) : UTBotBaseAction("Show") {

    override fun actionPerformed(e: AnActionEvent) {
        val virtualFile = LocalFileSystem.getInstance().findFileByNioFile(path)
            ?: error("Focus action should be disabled for path $path")

        val project = e.activeProject()
        val psiManager = PsiManager.getInstance(project)


        if (virtualFile.isDirectory) {
            psiManager.findDirectory(virtualFile)?.navigate(true)
        } else {
            psiManager.findFile(virtualFile)?.navigate(true)
        }
    }

    override fun updateIfEnabled(e: AnActionEvent) {
        e.presentation.isEnabledAndVisible = LocalFileSystem.getInstance().findFileByNioFile(path) != null
    }
}
