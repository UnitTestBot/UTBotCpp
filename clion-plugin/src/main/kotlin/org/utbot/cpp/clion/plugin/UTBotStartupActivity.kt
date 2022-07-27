package org.utbot.cpp.clion.plugin

import com.intellij.ide.util.RunOnceUtil
import com.intellij.openapi.components.service
import com.intellij.openapi.project.Project
import com.intellij.openapi.startup.StartupActivity
import org.utbot.cpp.clion.plugin.client.Client
import org.utbot.cpp.clion.plugin.settings.UTBotPluginSpecificSettings
import org.utbot.cpp.clion.plugin.settings.pluginSettings
import org.utbot.cpp.clion.plugin.settings.settings
import org.utbot.cpp.clion.plugin.ui.wizard.UTBotWizard
import org.utbot.cpp.clion.plugin.utils.getClient
import org.utbot.cpp.clion.plugin.utils.invokeOnEdt

class UTBotStartupActivity : StartupActivity {
    override fun runActivity(project: Project) {

        project.getClient()
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

    private fun guessPathsOnFirstProjectOpen(project: Project) {
        RunOnceUtil.runOnceForProject(project, "Guess UTBot paths in settings") {
            project.settings.predictPaths()
        }
    }
}