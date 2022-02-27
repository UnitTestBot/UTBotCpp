package org.utbot.cpp.clion.plugin.actions

import com.intellij.notification.Notification
import com.intellij.notification.NotificationAction
import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.vfs.LocalFileSystem
import com.intellij.psi.PsiManager
import java.nio.file.Path

class FocusAction(val path: Path) : NotificationAction("Show") {
    override fun actionPerformed(e: AnActionEvent) {
        val virtualFile = LocalFileSystem.getInstance().findFileByNioFile(path) ?: return
        if (virtualFile.isDirectory) {
            PsiManager.getInstance(e.project!!).findDirectory(virtualFile)?.navigate(true)
        } else {
            PsiManager.getInstance(e.project!!).findFile(virtualFile)?.navigate(true)
        }
    }

    override fun actionPerformed(e: AnActionEvent, notification: Notification) {
        actionPerformed(e)
    }
}
