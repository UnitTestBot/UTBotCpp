package org.utbot.cpp.clion.plugin.settings

import com.intellij.openapi.components.PersistentStateComponent
import com.intellij.openapi.components.Service
import com.intellij.openapi.components.State
import com.intellij.openapi.components.Storage
import com.intellij.openapi.project.Project
import org.utbot.cpp.clion.plugin.ui.targetsToolWindow.UTBotTarget

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
        var projectPath: String? = null,
        var buildDirRelativePath: String = "build-utbot",
        var testDirPath: String = "",
        var targetPath: String = UTBotTarget.autoTarget.path,
        var remotePath: String = "",
        var sourceDirs: Set<String> = setOf(),
        var cmakeOptions: String = DEFAULT_CMAKE_OPTIONS.joinToString(" "),
        var generateForStaticFunctions: Boolean = true,
        var useStubs: Boolean = true,
        var useDeterministicSearcher: Boolean = true,
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

    companion object {
        val DEFAULT_CMAKE_OPTIONS = listOf("-DCMAKE_EXPORT_COMPILE_COMMANDS=ON", "-DCMAKE_EXPORT_LINK_COMMANDS=ON")
    }
}