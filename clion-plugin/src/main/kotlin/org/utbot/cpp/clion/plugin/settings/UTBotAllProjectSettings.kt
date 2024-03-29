package org.utbot.cpp.clion.plugin.settings

import com.intellij.openapi.components.Service
import com.intellij.openapi.components.service
import com.intellij.openapi.project.Project
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.grpc.IllegalPathException
import org.utbot.cpp.clion.plugin.listeners.PluginActivationListener
import org.utbot.cpp.clion.plugin.listeners.UTBotSettingsChangedListener
import org.utbot.cpp.clion.plugin.ui.utbotToolWindow.targetToolWindow.UTBotTarget
import org.utbot.cpp.clion.plugin.utils.convertToRemotePathIfNeeded
import org.utbot.cpp.clion.plugin.utils.isWindows
import org.utbot.cpp.clion.plugin.utils.path
import java.io.File
import java.nio.file.InvalidPathException
import java.nio.file.Path
import java.nio.file.Paths

@Service
class UTBotAllProjectSettings(val project: Project) {
    val storedSettings: UTBotProjectStoredSettings
        get() = project.service<UTBotProjectStoredSettings>()

    val buildDirPath: Path
        get() {
            try {
                return Paths.get(project.path).resolve(storedSettings.buildDirRelPath)
            } catch (e: InvalidPathException) {
                throw IllegalPathException(
                    UTBot.message(
                        "paths.invalid",
                        "relative path to build dir",
                        storedSettings.buildDirRelPath
                    ),
                    storedSettings.buildDirRelPath
                )
            }
        }

    val testsDirPath: Path
        get() {
            try {
                return Paths.get(project.path).resolve(storedSettings.testDirRelativePath)
            } catch (e: InvalidPathException) {
                throw IllegalPathException(
                    storedSettings.testDirRelativePath,
                    UTBot.message(
                        "paths.invalid",
                        "relative path to tests dir",
                        storedSettings.testDirRelativePath
                    )
                )
            }
        }

    /**
     * If this property returns true, plugin must convert path sent and returned from server.
     * @see [String.convertToRemotePathIfNeeded] and [String.convertFromRemotePathIfNeeded]
     *
     * If we are on Windows, this is not a server, so it is always a remote scenario.
     */
    val isRemoteScenario: Boolean
        get() {
            val isLocalHost =
                projectIndependentSettings.serverName == "localhost" || projectIndependentSettings.serverName == "127.0.0.1"
            return !(storedSettings.remotePath == UTBotProjectStoredSettings.REMOTE_PATH_VALUE_FOR_LOCAL_SCENARIO && isLocalHost) || isWindows
        }

    fun fireUTBotSettingsChanged() {
        project.messageBus.let { bus ->
            if (!bus.isDisposed)
                bus.syncPublisher(UTBotSettingsChangedListener.TOPIC).settingsChanged(this)
        }
    }

    fun fireUTBotEnabledStateChanged() {
        project.messageBus.let {
            if (!it.isDisposed)
                project.messageBus.syncPublisher(PluginActivationListener.TOPIC).enabledChanged(project.settings.storedSettings.isPluginEnabled)
        }
    }

    fun predictPaths() {
        storedSettings.remotePath = UTBotProjectStoredSettings.REMOTE_PATH_VALUE_FOR_LOCAL_SCENARIO
        storedSettings.buildDirRelPath = UTBotProjectStoredSettings.DEFAULT_RELATIVE_PATH_TO_BUILD_DIR
        storedSettings.targetPath = UTBotTarget.autoTarget.path
    }

    companion object {
        const val DEFAULT_HOST = "localhost"
        const val DEFAULT_PORT = 2121
    }
}

data class UTBotSettingsModel(
    val projectSettings: UTBotProjectStoredSettings.State,
    val globalSettings: UTBotProjectIndependentSettings.State,
)
