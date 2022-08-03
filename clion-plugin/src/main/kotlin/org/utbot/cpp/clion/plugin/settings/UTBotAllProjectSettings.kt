package org.utbot.cpp.clion.plugin.settings

import com.intellij.openapi.components.Service
import com.intellij.openapi.components.service
import com.intellij.openapi.project.Project
import com.jetbrains.cidr.cpp.execution.CMakeAppRunConfiguration
import org.utbot.cpp.clion.plugin.listeners.UTBotSettingsChangedListener
import org.utbot.cpp.clion.plugin.ui.targetsToolWindow.UTBotTarget
import org.utbot.cpp.clion.plugin.utils.convertToRemotePathIfNeeded
import org.utbot.cpp.clion.plugin.utils.isWindows
import org.utbot.cpp.clion.plugin.utils.notifyWarning
import java.io.File
import java.nio.file.Path
import java.nio.file.Paths

@Service
class UTBotAllProjectSettings(val project: Project) {
    val storedSettings: UTBotProjectStoredSettings.State
        get() = project.service<UTBotProjectStoredSettings>().state

    // todo: maybe remove this property and access directly
    var projectPath: String
        get() {
            return storedSettings.projectPath
        }
        set(value) {
            storedSettings.projectPath = value
        }

    val buildDirPath: Path
        get() = Paths.get(projectPath).resolve(storedSettings.buildDirRelativePath)

    val convertedSourcePaths: List<String>
        get() = storedSettings.sourceDirs.map { it.convertToRemotePathIfNeeded(project) }

    val convertedTestDirPath: String
        get() = storedSettings.testDirPath.convertToRemotePathIfNeeded(project)

    val convertedTargetPath: String
        get() = if (storedSettings.targetPath == UTBotTarget.autoTarget.path) storedSettings.targetPath
        else storedSettings.targetPath.convertToRemotePathIfNeeded(project)

    val convertedProjectPath: String get() = projectPath.convertToRemotePathIfNeeded(project)

    /**
     * If this property returns true, plugin must convert path sent and returned from server.
     * @see [String.convertToRemotePathIfNeeded] and [String.convertFromRemotePathIfNeeded]
     *
     * If we are on Windows, this is not a server, so it is always a remote scenario.
     */
    val isRemoteScenario: Boolean
        get() {
            val isLocalHost =
                projectIndependentSettings.serverName == "localhost" || projectIndependentSettings.serverName == "127.0.0.01"
            return !(storedSettings.remotePath == projectPath && isLocalHost) || isWindows
        }

    fun fireUTBotSettingsChanged() {
        project.messageBus.syncPublisher(UTBotSettingsChangedListener.TOPIC).settingsChanged(this)
    }

    fun predictPaths() {
        fun getSourceFoldersFromSources(sources: Collection<File>) = sources.map { it.parent }.toMutableSet()

        storedSettings.remotePath = UTBotProjectStoredSettings.REMOTE_PATH_VALUE_FOR_LOCAL_SCENARIO
        storedSettings.buildDirRelativePath = UTBotProjectStoredSettings.DEFAULT_RELATIVE_PATH_TO_BUILD_DIR
        storedSettings.targetPath = UTBotTarget.autoTarget.path

        try {
            storedSettings.testDirPath =
                Paths.get(projectPath, UTBotProjectStoredSettings.DEFAULT_RELATIVE_PATH_TO_TEST_DIR).toString()
        } catch (e: IllegalStateException) {
            notifyWarning("Guessing settings failed: could not guess project path! Please specify it in settings!")
        }

        val cmakeRunConfiguration = CMakeAppRunConfiguration.getSelectedConfigurationAndTarget(project)?.first
        val buildConfigurationSources = cmakeRunConfiguration?.cMakeTarget?.buildConfigurations?.map { it.sources }
        //TODO: why do we use firstOrNull here?
        val cmakeConfiguration = buildConfigurationSources?.firstOrNull() ?: emptySet()

        storedSettings.sourceDirs = getSourceFoldersFromSources(cmakeConfiguration)
    }

    companion object {
        const val DEFAULT_HOST = "localhost"
        const val DEFAULT_PORT = 2121
    }
}

data class UTBotSettingsModel(
    var projectSettings: UTBotProjectStoredSettings.State,
    var globalSettings: UTBotProjectIndependentSettings.State,
)
