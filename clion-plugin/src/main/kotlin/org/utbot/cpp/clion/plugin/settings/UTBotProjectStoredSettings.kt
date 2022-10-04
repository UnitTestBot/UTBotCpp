package org.utbot.cpp.clion.plugin.settings

import com.intellij.ide.util.RunOnceUtil
import com.intellij.openapi.application.ApplicationManager
import com.intellij.openapi.components.PersistentStateComponent
import com.intellij.openapi.components.Service
import com.intellij.openapi.components.State
import com.intellij.openapi.components.Storage
import com.intellij.openapi.components.service
import com.intellij.openapi.project.Project
import org.utbot.cpp.clion.plugin.ui.utbotToolWindow.targetToolWindow.UTBotTarget
import org.utbot.cpp.clion.plugin.ui.utbotToolWindow.targetToolWindow.UTBotTargetsController
import org.utbot.cpp.clion.plugin.ui.wizard.UTBotWizard
import org.utbot.cpp.clion.plugin.utils.invokeOnEdt
import org.utbot.cpp.clion.plugin.utils.path
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
        var buildDirRelativePath: String = DEFAULT_RELATIVE_PATH_TO_BUILD_DIR,
        var testsDirRelativePath: String = DEFAULT_TESTS_DIR_RELATIVE_PATH,
        var targetPath: String = UTBotTarget.autoTarget.path,
        var remotePath: String = REMOTE_PATH_VALUE_FOR_LOCAL_SCENARIO,
        var sourceDirs: Set<String> = setOf(),
        var cmakeOptions: String = DEFAULT_CMAKE_OPTIONS.joinToString(" "),
        var generateForStaticFunctions: Boolean = true,
        var useStubs: Boolean = true,
        var useDeterministicSearcher: Boolean = false,
        var verbose: Boolean = false,
        var timeoutPerFunction: Int = 0,
        var timeoutPerTest: Int = 0,
        var isPluginEnabled: Boolean = false
    ) {
        fun fromSettingsModel(model: UTBotSettingsModel) {
            buildDirRelativePath = model.projectSettings.buildDirRelativePath
            testsDirRelativePath = model.projectSettings.testsDirRelativePath
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

    var cmakeOptions: String
        get() = myState.cmakeOptions
        set(value) {
            myState.cmakeOptions = value
        }

    var generateForStaticFunctions: Boolean
        get() = myState.generateForStaticFunctions
        set(value) {
            myState.generateForStaticFunctions = value
        }

    var useStubs: Boolean
        get() = myState.useStubs
        set(value) {
            myState.useStubs = value
        }

    var useDeterministicSearcher: Boolean
        get() = myState.useDeterministicSearcher
        set(value) {
            myState.useDeterministicSearcher = value
        }

    var verbose: Boolean
        get() = myState.verbose
        set(value) {
            myState.verbose = value
        }

    var timeoutPerFunction: Int
        get() = myState.timeoutPerFunction
        set(value) {
            myState.timeoutPerFunction = value
        }

    var timeoutPerTest: Int
        get() = myState.timeoutPerTest
        set(value) {
            myState.timeoutPerTest = value
        }

    var testDirRelativePath: String
        get() = myState.testsDirRelativePath
        set(value) {
            myState.testsDirRelativePath = value
        }

    var remotePath: String
        get() = myState.remotePath
        set(value) {
            myState.remotePath = value
        }

    var targetPath: String
        get() {
            if (isTargetUpToDate())
                return myState.targetPath
            return UTBotTarget.autoTarget.path
        }
        set(value) {
            myState.targetPath = value
        }

    val uiTargetPath: String
        get() = if (targetPath == UTBotTarget.autoTarget.path)
            UTBotTarget.autoTarget.path
        else
            Paths.get(project.path).relativize(Paths.get(targetPath)).toString()

    var buildDirRelativePath: String
        get() = myState.buildDirRelativePath
        set(value) {
            myState.buildDirRelativePath = value
        }

    var isPluginEnabled: Boolean
        get() = myState.isPluginEnabled
        set(value) {
            myState.isPluginEnabled = value
            if (myState.isPluginEnabled) {
                onPluginEnabled()
            }
        }

    var sourceDirs: Set<String> get() {
        return state.sourceDirs
    }
    set(value) {
        state.sourceDirs = value
    }

    private fun isTargetUpToDate(): Boolean {
        return project.service<UTBotTargetsController>().isTargetUpToDate(myState.targetPath)
    }



    override fun getState() = myState
    override fun loadState(state: State) {
        myState = state
    }

    // called when during component initialization if there is no persisted state.
    // See java docs for PersistingStateComponent
    override fun noStateLoaded() {
        myState.remotePath = REMOTE_PATH_VALUE_FOR_LOCAL_SCENARIO
    }

    private fun onPluginEnabled() {
        if (!ApplicationManager.getApplication().isUnitTestMode) {
            RunOnceUtil.runOnceForProject(project, "Show UTBot quick-start wizard to configure project") {
                invokeOnEdt {
                    UTBotWizard(project).showAndGet()
                }
            }
        }
    }

    companion object {
        val DEFAULT_CMAKE_OPTIONS = listOf("-DCMAKE_EXPORT_COMPILE_COMMANDS=ON", "-DCMAKE_EXPORT_LINK_COMMANDS=ON")

        // local means no conversion of paths is needed. This is the case for when server runs locally on Linux
        const val DEFAULT_TESTS_DIR_RELATIVE_PATH = "tests"
        const val REMOTE_PATH_VALUE_FOR_LOCAL_SCENARIO = ""
        const val DEFAULT_RELATIVE_PATH_TO_BUILD_DIR = "build"
        const val TIMEOUT_PER_TEST_MAX_VALUE = 1000
        const val TIMEOUT_PER_TEST_MIN_VALUE = 0
        const val TIMEOUT_PER_FUNCTION_MAX_VALUE = 1000
        const val TIMEOUT_PER_FUNCTION_MIN_VALUE = 0
    }
}
