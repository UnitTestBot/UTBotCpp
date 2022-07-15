package com.huawei.utbot.cpp.services

import com.huawei.utbot.cpp.messaging.SourceFoldersListener
import com.huawei.utbot.cpp.utils.relativize
import com.huawei.utbot.cpp.utils.notifyError
import com.huawei.utbot.cpp.messaging.UTBotSettingsChangedListener
import com.intellij.ide.util.RunOnceUtil
import com.intellij.openapi.components.PersistentStateComponent
import com.intellij.openapi.components.State
import com.intellij.openapi.diagnostic.Logger
import com.intellij.openapi.project.Project
import com.jetbrains.cidr.cpp.execution.CMakeAppRunConfiguration
import com.huawei.utbot.cpp.models.UTBotTarget
import com.huawei.utbot.cpp.utils.isWindows
import com.intellij.openapi.components.Storage
import org.apache.commons.io.FilenameUtils
import java.io.File
import java.nio.file.Path
import java.nio.file.Paths

/**
 * ProjectSettings is a service used to get project related info
 * for generating tests.
 *
 * @see UTBotConfigurable
 */
@State(
    name = "UtBotSettings",
    storages = [Storage("utbot-settings.xml")]
)
data class UTBotSettings(
    @com.intellij.util.xmlb.annotations.Transient
    val project: Project? = null,
) : PersistentStateComponent<UTBotSettings.State> {
    @com.intellij.util.xmlb.annotations.Transient
    val logger = Logger.getInstance(this::class.java)

    private var myState = State()

    init {
        // when user launches the project for the first time, try to predict paths
        project?.let {
            RunOnceUtil.runOnceForProject(
                project, "Predict UTBot paths"
            ) { predictPaths() }
        }
    }

    // serialized by the ide, the settings of plugin
    data class State(
        var targetPath: String = UTBotTarget.autoTarget.path,
        var buildDirRelativePath: String = "build-utbot",
        var testDirPath: String = "/",
        var remotePath: String = "",
        var sourceDirs: MutableSet<String> = mutableSetOf(),
        var port: Int = DEFAULT_PORT,
        var serverName: String = DEFAULT_HOST,
        var cmakeOptions: List<String> = DEFAULT_CMAKE_OPTIONS
    )

    var targetPath: String
        get() = state.targetPath
        set(value) {
            state.targetPath = value
        }

    val buildDirPath: Path
        get() = Paths.get(projectPath).resolve(state.buildDirRelativePath)

    var buildDirRelativePath: String
        get() = state.buildDirRelativePath
        set(value) {
            state.buildDirRelativePath = value
        }

    var testDirPath: String
        get() = state.testDirPath
        set(value) {
            state.testDirPath = value
        }

    var remotePath: String
        get() = state.remotePath
        set(value) {
            state.remotePath = value
        }

    var sourceDirs: Set<String>
        get() {
            return state.sourceDirs
        }
        set(value) {
            state.sourceDirs = value.toMutableSet()
            project?.messageBus?.syncPublisher(SourceFoldersListener.TOPIC)?.sourceFoldersChanged(value)
        }

    var port: Int
        get() = state.port
        set(value) {
            state.port = value
        }

    var serverName: String
        get() = state.serverName
        set(value) {
            state.serverName = value
        }

    var cmakeOptions: String
        get() = state.cmakeOptions.joinToString(" ")
        set(value) {
            state.cmakeOptions = value.split(" ")
        }

    val convertedSourcePaths: List<String>
        get() = sourceDirs.map { convertToRemotePathIfNeeded(it) }

    val convertedTestDirPath: String
        get() = convertToRemotePathIfNeeded(testDirPath)

    val convertedTargetPath: String
        get() = if (targetPath == UTBotTarget.autoTarget.path)
            targetPath
        else convertToRemotePathIfNeeded(targetPath)

    val convertedProjectPath: String
        get() {
            return convertToRemotePathIfNeeded(projectPath)
        }

    val projectPath: String
        get() {
            return project?.basePath ?: error("Could not get project path from project instance!")
        }

   private fun isLocalHost() = serverName == "localhost" || serverName == "127.0.0.1"

    fun isRemoteScenario() = !((remotePath == projectPath && isLocalHost()) || isWindows())

    /**
     * Convert absolute path on this machine to corresponding absolute path on remote host
     * if path to project on a remote machine was specified in the settings.
     *
     * If [isRemoteScenario] == false, this function returns [path] unchanged.
     *
     * @param path - absolute path on local machine to be converted
     */
    fun convertToRemotePathIfNeeded(path: String): String {
        logger.info("Converting $path to remote version")
        var result = path
        if (isRemoteScenario()) {
            val relativeToProjectPath = path.getRelativeToProjectPath()
            result = FilenameUtils.separatorsToUnix(Paths.get(remotePath, relativeToProjectPath).toString())
        }
        logger.info("The resulting path: $result")
        return result
    }

    /**
     * Convert absolute path on docker container to corresponding absolute path on local machine.
     *
     * If remote path == "", this function returns [path] unchanged.
     *
     * @param path - absolute path on docker to be converted
     */
    fun convertFromRemotePathIfNeeded(path: String): String {
        logger.info("Converting $path to local version")
        var result = path
        if (isRemoteScenario()) {
            val projectLocalPath = project?.basePath ?: let {
                notifyError("Could not get project path.", project)
                error("Project path is null when converting paths!")
            }
            val relativeToProjectPath = path.getRelativeToProjectPath(remotePath)
            result = FilenameUtils.separatorsToSystem(Paths.get(projectLocalPath, relativeToProjectPath).toString())
        }
        logger.info("The resulting path: $result")
        return result
    }

    private fun String.getRelativeToProjectPath(projectPath: String? = project?.basePath): String {
        logger.info("getRelativeToProjectPath was called on $this")
        projectPath ?: let {
            notifyError("Could not get project path.", project)
            error("Project path is null when converting paths!")
        }
        return relativize(projectPath, this)
    }

    // try to predict build dir, tests dir, cmake target paths, and get source folders paths from ide,
    // so user don't have to fill in them by hand
    fun predictPaths() {
        logger.info("predict paths was called")

        fun getSourceFoldersFromSources(sources: Collection<File>) = sources.map {
            it.parent
        }.toMutableSet()

        val projectPath = project?.basePath ?: return notifyError("Path to project unavailable", project)

        testDirPath = Paths.get(projectPath, "tests").toString()
        buildDirRelativePath = "build-utbot"
        targetPath = UTBotTarget.autoTarget.path

        val cmakeConfiguration = CMakeAppRunConfiguration.getSelectedConfigurationAndTarget(project)
            ?.first?.cMakeTarget?.buildConfigurations?.first()
            ?: return

        sourceDirs = getSourceFoldersFromSources(cmakeConfiguration.sources)
    }

    fun fireUTBotSettingsChanged() {
        project ?: return
        project.messageBus.syncPublisher(UTBotSettingsChangedListener.TOPIC).settingsChanged(this)
    }

    override fun getState() = myState

    override fun loadState(state: State) {
        myState = state
    }

    companion object {
        const val DEFAULT_HOST = "localhost"
        const val DEFAULT_PORT = 2121
        const val clientVersion = "2022.7"
        val DEFAULT_CMAKE_OPTIONS = listOf("-DCMAKE_EXPORT_COMPILE_COMMANDS=ON", "-DCMAKE_EXPORT_LINK_COMMANDS=ON")
    }
}
