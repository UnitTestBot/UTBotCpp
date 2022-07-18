package org.utbot.cpp.clion.plugin.ui.targetsToolWindow

import com.intellij.openapi.diagnostic.Logger
import com.intellij.openapi.project.Project
import com.intellij.openapi.wm.ToolWindow
import com.intellij.openapi.wm.ToolWindowFactory

class UTBotTargetsToolWindowFactory : ToolWindowFactory {
    private val logger = Logger.getInstance(this::class.java)

    override fun createToolWindowContent(project: Project, toolWindow: ToolWindow) {
        logger.info("createToolWindowContent was called")
        val contentManager = toolWindow.contentManager
        val content = contentManager.factory.createContent(
            UTBotTargetsController(project).createTargetsToolWindow(), null, false
        )
        toolWindow.contentManager.addContent(content)
    }
}
