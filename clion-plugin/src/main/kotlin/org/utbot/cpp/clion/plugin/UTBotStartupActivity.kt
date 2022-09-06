package org.utbot.cpp.clion.plugin

import com.intellij.ide.util.RunOnceUtil
import com.intellij.openapi.application.ApplicationManager
import com.intellij.openapi.components.service
import com.intellij.openapi.project.Project
import com.intellij.openapi.startup.StartupActivity
import com.intellij.openapi.wm.impl.status.widget.StatusBarWidgetsManager
import org.utbot.cpp.clion.plugin.client.ManagedClient
import org.utbot.cpp.clion.plugin.listeners.PluginActivationListener
import org.utbot.cpp.clion.plugin.settings.settings
import org.utbot.cpp.clion.plugin.ui.statusBar.VerboseModeWidgetFactory
import org.utbot.cpp.clion.plugin.ui.wizard.UTBotWizard
import org.utbot.cpp.clion.plugin.utils.invokeOnEdt
import org.utbot.cpp.clion.plugin.utils.projectLifetimeDisposable

class UTBotStartupActivity : StartupActivity {
    override fun runActivity(project: Project) {
        subscribeToUTBotDumpEvents(project)
        // We initialize Client here, so that initialization will not happen
        // when user issues first generation request which would cause a UI freeze.
        if (project.settings.storedSettings.isPluginEnabled) {
            initializeClient(project)
            guessPathsOnFirstOpen(project)
            showWizardOnFirstOpen(project)
        }
    }

    // Here we address the service ClientManager for the first time so that it
    // will be initialized by the ide and Client will be created.
    // Client in turn will create a grpc channel and start heart-beating the server.
    private fun initializeClient(project: Project) = project.service<ManagedClient>()


    private fun showWizardOnFirstOpen(project: Project) {
        if (!ApplicationManager.getApplication().isUnitTestMode) {
            invokeOnEdt {
                RunOnceUtil.runOnceForProject(project, "Show UTBot quick-start wizard to configure project") {
                    UTBotWizard(project).showAndGet()
                }
            }
        }
    }

    /**
     * Makes subscriptions to events, that should work even if plugin is disabled
     */
    private fun subscribeToUTBotDumpEvents(project: Project) {
        with(project.messageBus.connect(project.projectLifetimeDisposable)) {
            subscribe(PluginActivationListener.TOPIC, PluginActivationListener {
                project.service<StatusBarWidgetsManager>().updateWidget(VerboseModeWidgetFactory::class.java)
            })
        }
    }

    private fun guessPathsOnFirstOpen(project: Project) {
        RunOnceUtil.runOnceForProject(project, "Guess UTBot paths in settings") {
            project.settings.predictPaths()
        }
    }
}
