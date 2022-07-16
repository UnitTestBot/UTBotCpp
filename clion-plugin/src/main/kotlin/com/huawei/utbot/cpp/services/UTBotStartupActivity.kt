package com.huawei.utbot.cpp.services

import com.huawei.utbot.cpp.client.Client
import com.huawei.utbot.cpp.ui.wizard.UTBotWizard
import com.huawei.utbot.cpp.utils.getClient
import com.huawei.utbot.cpp.utils.invokeOnEdt
import com.huawei.utbot.cpp.utils.utbotSettings
import com.intellij.ide.util.RunOnceUtil
import com.intellij.openapi.components.service
import com.intellij.openapi.project.Project
import com.intellij.openapi.startup.StartupActivity

class UTBotStartupActivity : StartupActivity {
    override fun runActivity(project: Project) {
        // start plugin and connect to server on project opening
        project.getClient()
        guessPathsOnFirstProjectOpen(project)
        showWizardOnFirstProjectOpen(project)
    }

    private fun showWizardOnFirstProjectOpen(project: Project) {
        val pluginSettings = service<UTBotPluginSpecificSettings>()
        if (pluginSettings.isFirstLaunch && !Client.IS_TEST_MODE) {
            pluginSettings.isFirstLaunch = false
            invokeOnEdt {
                UTBotWizard(project).showAndGet()
            }
        }
    }

    private fun guessPathsOnFirstProjectOpen(project: Project) {
        RunOnceUtil.runOnceForProject(project, "Guess UTBot paths in settings") {
            project.utbotSettings.predictPaths()
        }
    }
}
