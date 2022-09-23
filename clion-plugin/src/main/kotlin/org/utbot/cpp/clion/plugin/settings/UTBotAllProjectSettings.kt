package org.utbot.cpp.clion.plugin.settings

import com.intellij.openapi.components.Service
import com.intellij.openapi.components.service
import com.intellij.openapi.project.Project
import org.utbot.cpp.clion.plugin.listeners.UTBotSettingsChangedListener
import org.utbot.cpp.clion.plugin.ui.utbotToolWindow.targetToolWindow.UTBotTarget
import org.utbot.cpp.clion.plugin.utils.convertToRemotePathIfNeeded
import org.utbot.cpp.clion.plugin.utils.isWindows
import org.utbot.cpp.clion.plugin.utils.path
import java.io.File
import java.nio.file.Path
import java.nio.file.Paths

@Service
class UTBotAllProjectSettings(val project: Project) {
    val storedSettings: UTBotProjectStoredSettings.State
        get() = project.service<UTBotProjectStoredSettings>().state

    val buildDirPath: Path
        get() = Paths.get(project.path).resolve(storedSettings.buildDirRelativePath)

    val testsDirPath: Path
        get() = Paths.get(project.path).resolve(storedSettings.testsDirRelativePath)

    val convertedSourcePaths: List<String>
        get() = storedSettings.sourceDirs.map { it.convertToRemotePathIfNeeded(project) }

    val convertedTestDirPath: String
        get() = testsDirPath.toString().convertToRemotePathIfNeeded(project)

    val convertedTargetPath: String
        get() = if (storedSettings.targetPath == UTBotTarget.autoTarget.path) storedSettings.targetPath
        else storedSettings.targetPath.convertToRemotePathIfNeeded(project)

    val convertedProjectPath: String get() = project.path.convertToRemotePathIfNeeded(project)

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
            if(!bus.isDisposed)
                bus.syncPublisher(UTBotSettingsChangedListener.TOPIC).settingsChanged(this)
        }
    }

    fun predictPaths() {
        fun getSourceFoldersFromSources(sources: Collection<File>) = sources.map { it.parent }.toMutableSet()

        storedSettings.remotePath = UTBotProjectStoredSettings.REMOTE_PATH_VALUE_FOR_LOCAL_SCENARIO
        storedSettings.buildDirRelativePath = UTBotProjectStoredSettings.DEFAULT_RELATIVE_PATH_TO_BUILD_DIR
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
