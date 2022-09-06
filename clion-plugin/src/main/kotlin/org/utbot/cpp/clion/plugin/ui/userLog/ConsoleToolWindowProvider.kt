package org.utbot.cpp.clion.plugin.ui.userLog

import com.intellij.openapi.diagnostic.Logger
import com.intellij.openapi.project.Project
import com.intellij.openapi.wm.ToolWindow
import com.intellij.openapi.wm.ToolWindowFactory

class ConsoleToolWindowProvider : ToolWindowFactory {
    private val logger = Logger.getInstance(this::class.java)

    override fun createToolWindowContent(project: Project, toolWindow: ToolWindow) {
        logger.debug("createToolWindowContent was called")

        val contentManager = toolWindow.contentManager
        val content =
            contentManager.factory.createContent(ConsoleToolWindow(project), null, false)
        contentManager.addContent(content)
    }
}
