package org.utbot.cpp.clion.plugin.ui

import com.intellij.openapi.project.Project
import com.intellij.openapi.wm.ToolWindow
import com.intellij.openapi.wm.ToolWindowFactory
import org.utbot.cpp.clion.plugin.listeners.PluginActivationListener
import org.utbot.cpp.clion.plugin.settings.settings
import org.utbot.cpp.clion.plugin.utils.client
import org.utbot.cpp.clion.plugin.utils.invokeOnEdt

/**
 * Base class for creating tool windows that appear/disappear
 * when plugin is enabled/disabled.
 */
abstract class UTBotAwareToolWindowFactory : ToolWindowFactory {
    abstract fun createToolWindow(project: Project, toolWindow: ToolWindow)

    override fun createToolWindowContent(project: Project, toolWindow: ToolWindow) {
        createToolWindow(project, toolWindow)
        toolWindow.isAvailable = project.settings.storedSettings.isPluginEnabled
        project.messageBus.connect(project.client).subscribe(PluginActivationListener.TOPIC,
            PluginActivationListener { enabled: Boolean ->
                invokeOnEdt {
                    toolWindow.isAvailable = enabled
                }
            })
    }
}
