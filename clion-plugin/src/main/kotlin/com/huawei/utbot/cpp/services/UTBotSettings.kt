package com.huawei.utbot.cpp.services

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
import java.io.File
import java.nio.file.Paths

/**
 * ProjectSettings is a service used to get project related info
 * for generating tests.
 *
 * @see UTBotConfigurable
 */
@State(name = "UTBotProjectSettings")
data class UTBotSettings(
    @com.intellij.util.xmlb.annotations.Transient
    val project: Project? = null,
) : PersistentStateComponent<UTBotSettings.State> {
    @com.intellij.util.xmlb.annotations.Transient
    val logger = Logger.getInstance(this::class.java)

    private var myState = State()

    init {
        logger.info("ProjectSettings instance's constructor is called: project == $project")
        // when user launches the project for the first time, try to predict paths
        project?.let {
            RunOnceUtil.runOnceForProject(
                project, "Predict UTBot paths"
            ) { predictPaths() }
        }
    }

    // used for serializing by the ide
    data class State(
        var targetPath: String = UTBotTarget.autoTarget.path,
        var buildDirPath: String = "/",
        var testDirPath: String = "/",
        var synchronizeCode: Boolean = false,
        var remotePath: String = "",
        var sourcePaths: List<String> = emptyList(),
        var port: Int = 2121,
        var serverName: String = "localhost"
    )

    var targetPath: String
        get() = state.targetPath
        set(value) {
            state.targetPath = value
        }

    var buildDirPath: String
        get() = state.buildDirPath
        set(value) {
            state.buildDirPath = value
        }

    var testDirPath: String
        get() = state.testDirPath
        set(value) {
            state.testDirPath = value
        }

    var synchronizeCode: Boolean
        get() = state.synchronizeCode
        set(value) {
            state.synchronizeCode = value
        }

    var remotePath: String
        get() = state.remotePath
        set(value) {
            state.remotePath = value
        }

    var sourcePaths: List<String>
        get() = state.sourcePaths
        set(value) {
            state.sourcePaths = value
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

    val convertedSourcePaths: List<String>
        get() = sourcePaths.map { convertToRemotePathIfNeeded(it) }

    val relativeBuildDirPath: String
        get() = buildDirPath.getRelativeToProjectPath()

    val convertedBuildDirPath: String
        get() = convertToRemotePathIfNeeded(buildDirPath)

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

    fun isRemoteScenario() = remotePath.isNotEmpty()

    /**
     * Convert absolute path on this machine to corresponding absolute path on docker
     * if path to project on a remote machine was specified in the settings.
     *
     * If remote path == "", this function returns [path] unchanged.
     *
     * @param path - absolute path on local machine to be converted
     */
    fun convertToRemotePathIfNeeded(path: String): String {
        logger.info("Converting $path to remote version")
        var result = path
        if (isRemoteScenario()) {
            val relativeToProjectPath = path.getRelativeToProjectPath()
            result = Paths.get(remotePath, relativeToProjectPath).toString()
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
                return "/"
            }
            val relativeToProjectPath = path.getRelativeToProjectPath(remotePath)
            result = Paths.get(projectLocalPath, relativeToProjectPath).toString()
        }
        logger.info("The resulting path: $result")
        return result
    }

    private fun String.getRelativeToProjectPath(projectPath: String? = project?.basePath): String {
        logger.info("getRelativeToProjectPath was called on $this")
        projectPath ?: let {
            notifyError("Could not get project path.", project)
            return "/"
        }
        return relativize(projectPath, this)
    }

    private fun couldNotGetItem(itemName: String) = notifyError(
        """Could not get $itemName.
               Please, provide paths manually in settings -> tools -> UTBot Settings.
            """.trimMargin(),
        project
    )

    // try to predict build dir, tests dir, cmake target paths, and get source folders paths from ide,
    // so user don't have to fill in them by hand
    fun predictPaths() {
        logger.info("predict paths was called")

        fun getSourceFoldersFromSources(sources: Collection<File>) = sources.map {
            it.parent
        }.distinct()

        val projectPath = project?.basePath ?: return notifyError("Path to project unavailable", project)

        println("PREDICTING PATHS")
        println("projectPath: $projectPath")
        testDirPath = Paths.get(projectPath, "tests").toString()
        buildDirPath = Paths.get(projectPath, "build-utbot").toString()
        targetPath = UTBotTarget.autoTarget.path

        val cmakeConfiguration = CMakeAppRunConfiguration.getSelectedConfigurationAndTarget(project)
            ?.first?.cMakeTarget?.buildConfigurations?.first()
            ?: return notifyError("Can't get source paths: CMake model is unavailable")

        sourcePaths = getSourceFoldersFromSources(cmakeConfiguration.sources)
    }

    fun fireUTBotSettingsChanged() {
        project ?: return
        project.messageBus.syncPublisher(UTBotSettingsChangedListener.TOPIC).settingsChanged(this)
    }

    override fun getState() = myState

    override fun loadState(state: State) {
        myState = state
    }
}
