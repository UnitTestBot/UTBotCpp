package org.utbot.cpp.clion.plugin

import com.intellij.ide.util.RunOnceUtil
import com.intellij.openapi.components.service
import com.intellij.openapi.project.Project
import com.intellij.openapi.startup.StartupActivity
import org.utbot.cpp.clion.plugin.client.Client
import org.utbot.cpp.clion.plugin.client.ClientManager
import org.utbot.cpp.clion.plugin.settings.pluginSettings
import org.utbot.cpp.clion.plugin.settings.settings
import org.utbot.cpp.clion.plugin.ui.wizard.UTBotWizard
import org.utbot.cpp.clion.plugin.utils.getCurrentClient
import org.utbot.cpp.clion.plugin.utils.invokeOnEdt

class UTBotStartupActivity : StartupActivity {
    override fun runActivity(project: Project) {
        // we initialize Client here, so that initialization will not happen when user issues first
        // generation request which would cause a UI freeze
        initializeClient(project)
        guessPathsOnFirstProjectOpen(project)
        showWizardOnFirstProjectOpen(project)
    }

    private fun showWizardOnFirstProjectOpen(project: Project) {
        if (pluginSettings.isFirstLaunch && !Client.IS_TEST_MODE) {
            pluginSettings.isFirstLaunch = false
            invokeOnEdt {
                UTBotWizard(project).showAndGet()
            }
        }
    }

    private fun initializeClient(project: Project) {
        // Here we address the service ClientManager for the first time so that it
        // will be initialized by the ide and Client will be created.
        // Client in turn will create a grpc channel and start heartbeating the server
        project.service<ClientManager>()
    }

    private fun guessPathsOnFirstProjectOpen(project: Project) {
        RunOnceUtil.runOnceForProject(project, "Guess UTBot paths in settings") {
            project.settings.predictPaths()
        }
    }
}