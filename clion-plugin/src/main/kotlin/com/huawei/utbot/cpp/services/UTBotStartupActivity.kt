package com.huawei.utbot.cpp.services

import com.huawei.utbot.cpp.client.Client
import com.huawei.utbot.cpp.client.Client.Companion.IS_TEST_MODE
import com.huawei.utbot.cpp.ui.userLog.OutputProvider
import com.huawei.utbot.cpp.ui.wizard.UTBotWizard
import com.huawei.utbot.cpp.utils.getClient
import com.huawei.utbot.cpp.utils.invokeOnEdt
import com.intellij.ide.util.RunOnceUtil
import com.intellij.openapi.components.service
import com.intellij.openapi.project.Project
import com.intellij.openapi.startup.StartupActivity
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.swing.Swing
import org.koin.core.context.startKoin
import org.koin.dsl.module

class UTBotStartupActivity: StartupActivity {
    override fun runActivity(project: Project) {
        // start plugin and connect to server on project opening
        setupDependencies(project)
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

    private fun setupDependencies(project: Project) {
        val console = project.service<OutputProvider>().clientOutputChannel.outputConsole
        val clientDependencies = module {
            single { console }
            single { if (IS_TEST_MODE) Dispatchers.Default else Dispatchers.Swing }
        }
        startKoin {
            modules(clientDependencies)
        }
    }
}
