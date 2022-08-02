package org.utbot.cpp.clion.plugin.settings

import com.intellij.openapi.components.PersistentStateComponent
import com.intellij.openapi.components.Service
import com.intellij.openapi.components.State
import com.intellij.openapi.components.Storage
import com.intellij.openapi.project.Project
import com.intellij.openapi.project.guessProjectDir
import org.utbot.cpp.clion.plugin.ui.targetsToolWindow.UTBotTarget
import java.nio.file.Paths

/**
 * Settings that are specific to each project
 */
@Service
@State(
    name = "UTBotProjectStoredSettings",
    storages = [Storage("utbot-project-stored-settings.xml")]
)
class UTBotProjectStoredSettings(val project: Project) : PersistentStateComponent<UTBotProjectStoredSettings.State> {
    private var myState = State()

    // serialized by the ide
    data class State(
        var projectPath: String = "",
        var buildDirRelativePath: String = DEFAULT_RELATIVE_PATH_TO_BUILD_DIR,
        var testDirPath: String = "",
        var targetPath: String = UTBotTarget.autoTarget.path,
        var remotePath: String = REMOTE_PATH_VALUE_FOR_LOCAL_SCENARIO,
        var sourceDirs: Set<String> = setOf(),
        var cmakeOptions: String = DEFAULT_CMAKE_OPTIONS.joinToString(" "),
        var generateForStaticFunctions: Boolean = true,
        var useStubs: Boolean = true,
        var useDeterministicSearcher: Boolean = true,
        var isLocalOrWslScenario: Boolean = false,
        var verbose: Boolean = false,
        var timeoutPerFunction: Int = 0,
        var timeoutPerTest: Int = 30
    ) {
        fun fromSettingsModel(model: UTBotSettingsModel) {
            buildDirRelativePath = model.projectSettings.buildDirRelativePath
            testDirPath = model.projectSettings.testDirPath
            targetPath = model.projectSettings.targetPath
            remotePath = model.projectSettings.remotePath
            sourceDirs = model.projectSettings.sourceDirs
            cmakeOptions = model.projectSettings.cmakeOptions
            generateForStaticFunctions = model.projectSettings.generateForStaticFunctions
            useStubs = model.projectSettings.useStubs
            useDeterministicSearcher = model.projectSettings.useDeterministicSearcher
            verbose = model.projectSettings.verbose
            timeoutPerFunction = model.projectSettings.timeoutPerFunction
            timeoutPerTest = model.projectSettings.timeoutPerTest
        }
    }

    override fun getState() = myState
    override fun loadState(state: State) {
        myState = state
    }

    // called when during component initialization if there is no persisted state.
    // See java docs for PersistingStateComponent
    override fun noStateLoaded() {
        myState.projectPath =
            project.guessProjectDir()?.path ?: error("Could not guess project path! Should be specified in settings")
        myState.testDirPath = Paths.get(myState.projectPath).resolve(DEFAULT_RELATIVE_PATH_TO_TEST_DIR).toString()
        myState.remotePath = REMOTE_PATH_VALUE_FOR_LOCAL_SCENARIO
    }


    companion object {
        val DEFAULT_CMAKE_OPTIONS = listOf("-DCMAKE_EXPORT_COMPILE_COMMANDS=ON", "-DCMAKE_EXPORT_LINK_COMMANDS=ON")
        // local means no conversion of paths is needed. This is the case for when server runs locally on Linux
        const val REMOTE_PATH_VALUE_FOR_LOCAL_SCENARIO = ""
        const val DEFAULT_RELATIVE_PATH_TO_TEST_DIR = "utbot-tests"
        const val DEFAULT_RELATIVE_PATH_TO_BUILD_DIR = "utbot-build"
        const val TIMEOUT_PER_TEST_MAX_VALUE = 1000
        const val TIMEOUT_PER_TEST_MIN_VALUE = 0
        const val TIMEOUT_PER_FUNCTION_MAX_VALUE = 1000
        const val TIMEOUT_PER_FUNCTION_MIN_VALUE = 0
    }
}