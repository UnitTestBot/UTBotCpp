package org.utbot.cpp.clion.plugin

import com.intellij.ide.util.RunOnceUtil
import com.intellij.openapi.components.service
import com.intellij.openapi.project.Project
import com.intellij.openapi.startup.StartupActivity
import org.tinylog.kotlin.Logger
import org.utbot.cpp.clion.plugin.client.ManagedClient
import org.utbot.cpp.clion.plugin.settings.settings

class UTBotStartupActivity : StartupActivity {
    override fun runActivity(project: Project) {
        // We initialize Client here, so that initialization will not happen
        // when user issues first generation request which would cause a UI freeze.
        if (project.settings.storedSettings.isPluginEnabled) {
            Logger.info("startup activity is run: Initializing client")
            initializeClient(project)
            guessPathsOnFirstOpen(project)
        }
    }

    // Here we address the service ClientManager for the first time so that it
    // will be initialized by the ide and Client will be created.
    // Client in turn will create a grpc channel and start heart-beating the server.
    private fun initializeClient(project: Project) = project.service<ManagedClient>()


    private fun guessPathsOnFirstOpen(project: Project) {
        RunOnceUtil.runOnceForProject(project, "Guess UTBot paths in settings") {
            project.settings.predictPaths()
        }
    }
}
