package org.utbot.cpp.clion.plugin.settings

import com.intellij.openapi.project.Project
import org.utbot.cpp.clion.plugin.ui.targetsToolWindow.UTBotTarget
import com.intellij.openapi.components.PersistentStateComponent
import com.intellij.openapi.components.Service
import com.intellij.openapi.components.State
import com.intellij.openapi.components.Storage
import com.intellij.openapi.components.service
import com.intellij.openapi.diagnostic.Logger
import com.intellij.openapi.project.guessProjectDir
import com.jetbrains.cidr.cpp.execution.CMakeAppRunConfiguration
import org.utbot.cpp.clion.plugin.listeners.UTBotSettingsChangedListener
import org.utbot.cpp.clion.plugin.utils.convertToRemotePathIfNeeded
import org.utbot.cpp.clion.plugin.utils.isWindows
import org.utbot.cpp.clion.plugin.utils.notifyWarning
import java.io.File
import java.nio.file.Path
import java.nio.file.Paths

/**
 * Settings that are the same for all projects
 */
@Service
@State(
    name = "UtBotGlobalSettings",
    storages = [Storage("utbot-global-settings.xml")]
)
class UTBotGlobalSettings : PersistentStateComponent<UTBotGlobalSettings.State> {
    data class State(
        var port: Int = UTBotAllSettings.DEFAULT_PORT,
        var serverName: String = UTBotAllSettings.DEFAULT_HOST,
    )

    private var myState: State = State()
    override fun getState(): State = myState
    override fun loadState(state: State) {
        myState = state
    }
}


/**
 * Settings that are specific to each project
 */
@Service
@State(
    name = "UtBotSettings",
    storages = [Storage("utbot-settings.xml")]
)
class UTBotProjectSettings(val project: Project) : PersistentStateComponent<UTBotProjectSettings.State> {
    private var myState = State()

    // serialized by the ide
    data class State(
        var projectPath: String? = null,
        var buildDirRelativePath: String = "build-utbot",
        var testDirPath: String = "",
        var targetPath: String = UTBotTarget.autoTarget.path,
        var remotePath: String = "",
        var sourceDirs: Set<String> = setOf(),
        var cmakeOptions: List<String> = DEFAULT_CMAKE_OPTIONS,
        var generateForStaticFunctions: Boolean = true,
        var useStubs: Boolean = true,
        var useDeterministicSearcher: Boolean = true,
        var verbose: Boolean = false,
        var timeoutPerFunction: Int = 0,
        var timeoutPerTest: Int = 30
    )

    override fun getState() = myState

    override fun loadState(state: State) {
        myState = state
    }

    companion object {
        val DEFAULT_CMAKE_OPTIONS = listOf("-DCMAKE_EXPORT_COMPILE_COMMANDS=ON", "-DCMAKE_EXPORT_LINK_COMMANDS=ON")
    }
}

data class UTBotSettingsModel(
    var port: Int,
    var serverName: String,
    var projectPath: String,
    var buildDirRelativePath: String,
    var testDirPath: String,
    var targetPath: String,
    var remotePath: String,
    var sourceDirs: Set<String>,
    var cmakeOptions: String,
    var generateForStaticFunctions: Boolean,
    var useStubs: Boolean,
    var useDeterministicSearcher: Boolean,
    var verbose: Boolean,
    var timeoutPerFunction: Int,
    var timeoutPerTest: Int
)

@Service
class UTBotAllSettings(val project: Project) {
    private val logger = Logger.getInstance(this::class.java)
    private val projectSettings: UTBotProjectSettings.State get() = project.service<UTBotProjectSettings>().state
    private val globalSettings: UTBotGlobalSettings.State get() = service<UTBotGlobalSettings>().state

    var port: Int
        get() = globalSettings.port
        set(value) {
            globalSettings.port = value
        }

    var serverName: String
        get() = globalSettings.serverName
        set(value) {
            globalSettings.serverName = value
        }

    var projectPath: String
        get() {
            if (projectSettings.projectPath == null) {
                projectSettings.projectPath = project.guessProjectDir()?.path
                    ?: error("Could not guess project path! Should be specified in settings by user")
            }
            return projectSettings.projectPath!!
        }
        set(value) {
            projectSettings.projectPath = value
        }

    var buildDirRelativePath: String
        get() = projectSettings.buildDirRelativePath
        set(value) {
            projectSettings.buildDirRelativePath = value
        }

    var testDirPath: String
        get() = projectSettings.testDirPath
        set(value) {
            projectSettings.testDirPath = value
        }

    var targetPath: String
        get() = projectSettings.targetPath
        set(value) {
            projectSettings.targetPath = value
        }

    var remotePath: String
        get() = projectSettings.remotePath
        set(value) {
            projectSettings.remotePath = value
        }

    var sourceDirs: Set<String>
        get() = projectSettings.sourceDirs
        set(value) {
            projectSettings.sourceDirs = value
        }

    var cmakeOptions: String
        get() = projectSettings.cmakeOptions.joinToString(" ")
        set(value) {
            projectSettings.cmakeOptions = value.split(" ")
        }

    var generateForStaticFunctions: Boolean
        get() = projectSettings.generateForStaticFunctions
        set(value) {
            projectSettings.generateForStaticFunctions = value
        }

    var useStubs: Boolean
        get() = projectSettings.useStubs
        set(value) {
            projectSettings.useStubs = value
        }

    var useDeterministicSearcher: Boolean
        get() = projectSettings.useDeterministicSearcher
        set(value) {
            projectSettings.useDeterministicSearcher = value
        }

    var verbose: Boolean
        get() = projectSettings.verbose
        set(value) {
            projectSettings.verbose = value
        }

    var timeoutPerFunction: Int
        get() = projectSettings.timeoutPerFunction
        set(value) {
            projectSettings.timeoutPerFunction = value
        }

    var timeoutPerTest: Int
        get() = projectSettings.timeoutPerTest
        set(value) {
            projectSettings.timeoutPerTest = value
        }

    val convertedSourcePaths: List<String>
        get() = sourceDirs.map { it.convertToRemotePathIfNeeded(project) }

    val convertedTestDirPath: String
        get() = testDirPath.convertToRemotePathIfNeeded(project)

    val convertedTargetPath: String
        get() = if (targetPath == UTBotTarget.autoTarget.path)
            targetPath
        else targetPath.convertToRemotePathIfNeeded(project)

    val convertedProjectPath: String get() = projectPath.convertToRemotePathIfNeeded(project)

    //TODO: it seems to be a kind of boolshit to me
    private val isLocalHost: Boolean
        get() = serverName == "localhost" || serverName == "127.0.0.01"

    //TODO: it is unclear, requires a comment
    val isRemoteScenario: Boolean
        get() = !(remotePath == projectPath && isLocalHost) || isWindows

    fun predictPaths() {
        logger.info("predict paths was called")

        fun getSourceFoldersFromSources(sources: Collection<File>) = sources.map {
            it.parent
        }.toMutableSet()

        remotePath = projectPath
        try {
            testDirPath = Paths.get(projectPath, "tests").toString()
        } catch (e: IllegalStateException) {
            notifyWarning("Guessing settings failed: could not guess project path! Please specify project path in settings!")
        }
        buildDirRelativePath = "build-utbot"
        targetPath = UTBotTarget.autoTarget.path

        val cmakeConfiguration = CMakeAppRunConfiguration.getSelectedConfigurationAndTarget(project)
            ?.first?.cMakeTarget?.buildConfigurations?.first()
            ?: return

        sourceDirs = getSourceFoldersFromSources(cmakeConfiguration.sources)
    }

    val buildDirPath: Path
        get() = Paths.get(projectPath).resolve(buildDirRelativePath)

    fun asModel() = UTBotSettingsModel(
        port,
        serverName,
        projectPath,
        buildDirRelativePath,
        testDirPath,
        targetPath,
        remotePath,
        sourceDirs,
        cmakeOptions,
        generateForStaticFunctions,
        useStubs,
        useDeterministicSearcher,
        verbose,
        timeoutPerFunction,
        timeoutPerTest
    )

    fun applyModel(model: UTBotSettingsModel) {
        port = model.port
        serverName = model.serverName
        projectPath = model.projectPath
        buildDirRelativePath = model.buildDirRelativePath
        testDirPath = model.testDirPath
        targetPath = model.targetPath
        remotePath = model.remotePath
        sourceDirs = model.sourceDirs
        cmakeOptions = model.cmakeOptions
        generateForStaticFunctions = model.generateForStaticFunctions
        useStubs = model.useStubs
        useDeterministicSearcher = model.useDeterministicSearcher
        verbose = model.verbose
        timeoutPerFunction = model.timeoutPerFunction
        timeoutPerTest = model.timeoutPerTest
    }


    fun fireUTBotSettingsChanged() {
        project.messageBus.syncPublisher(UTBotSettingsChangedListener.TOPIC).settingsChanged(this)
    }

    companion object {
        const val clientVersion = "2022.7"
        const val DEFAULT_HOST = "localhost"
        const val DEFAULT_PORT = 2121
    }
}
