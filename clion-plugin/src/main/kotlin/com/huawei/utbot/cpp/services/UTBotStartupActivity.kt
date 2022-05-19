package com.huawei.utbot.cpp.services

import com.huawei.utbot.cpp.client.Client
import com.huawei.utbot.cpp.ui.wizard.UTBotWizard
import com.huawei.utbot.cpp.utils.getClient
import com.huawei.utbot.cpp.utils.invokeOnEdt
import com.intellij.ide.util.RunOnceUtil
import com.intellij.openapi.project.Project
import com.intellij.openapi.startup.StartupActivity

class UTBotStartupActivity: StartupActivity {
    override fun runActivity(project: Project) {
        // start plugin and connect to server on project opening
        project.getClient()
        showWizardOnFirstProjectOpen(project)
    }

    private fun showWizardOnFirstProjectOpen(project: Project) {
        RunOnceUtil.runOnceForProject(project, "Show UTBot Wizard") {
            if (!Client.IS_TEST_MODE) {
                invokeOnEdt {
                    UTBotWizard(project).showAndGet()
                }
            }
        }
    }
}
