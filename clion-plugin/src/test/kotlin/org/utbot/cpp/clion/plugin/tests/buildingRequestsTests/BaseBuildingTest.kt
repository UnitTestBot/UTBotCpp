package org.utbot.cpp.clion.plugin.tests.buildingRequestsTests

import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.ex.ActionUtil
import com.intellij.openapi.project.Project
import com.intellij.testFramework.fixtures.CodeInsightTestFixture
import org.junit.jupiter.api.AfterEach
import org.junit.jupiter.api.extension.ExtendWith
import org.utbot.cpp.clion.plugin.SwingEdtInterceptor
import org.utbot.cpp.clion.plugin.actions.generate.GenerateForLineAction
import org.utbot.cpp.clion.plugin.client.logger.SystemWriter
import org.utbot.cpp.clion.plugin.createActionEventFrom
import org.utbot.cpp.clion.plugin.createFixture
import org.utbot.cpp.clion.plugin.moveCursorToLine
import org.utbot.cpp.clion.plugin.settings.UTBotProjectStoredSettings
import org.utbot.cpp.clion.plugin.settings.settings
import org.utbot.cpp.clion.plugin.utils.logger
import testsgen.Testgen
import java.io.File
import java.nio.file.Path
import java.nio.file.Paths

@ExtendWith(SwingEdtInterceptor::class)
open class BaseBuildingTest {
    val projectPath: Path =
        Paths.get(File(".").canonicalPath).resolve("../integration-tests/c-example-mini").normalize()
    protected val testsDirRelativePath = "cl-plugin-test-tests"
    protected val testsDirectoryPath: Path = projectPath.resolve(testsDirRelativePath)
    protected val buildDirRelativePath = "build"
    protected val fixture: CodeInsightTestFixture = createFixture(projectPath)
    protected val project: Project
        get() = fixture.project
    val settings = project.settings.storedSettings

    init {
        settings.buildDirRelativePath = buildDirRelativePath
        settings.testDirRelativePath = projectPath.relativize(testsDirectoryPath).toString()
        settings.isPluginEnabled = true
        project.logger.logWriters.let {
            it.clear()
            it.add(SystemWriter())
        }
    }

    @AfterEach
    fun tearDown() {
        fixture.tearDown()
    }

    // HELPER METHODS:

    protected fun createExpectedProjectContext(remotePath: String): Testgen.ProjectContext {
        val isRemote = isRemoteScenarioExpectedInTests(remotePath)
        return Testgen.ProjectContext.newBuilder().apply {
            projectPath = if (isRemote) remotePath else this@BaseBuildingTest.projectPath.toString()
            projectName = project.name
            testDirPath =
                if (isRemote) "$remotePath/$testsDirRelativePath" else this@BaseBuildingTest.testsDirectoryPath.toString()
            buildDirRelativePath = this@BaseBuildingTest.buildDirRelativePath
        }.build()
    }

    protected fun createExpectedSettingsContextFromCurrentSettings(): Testgen.SettingsContext {
        return Testgen.SettingsContext.newBuilder().apply {
            generateForStaticFunctions = settings.generateForStaticFunctions
            verbose = settings.verbose
            timeoutPerFunction = settings.timeoutPerFunction
            timeoutPerTest = settings.timeoutPerTest
            useDeterministicSearcher = settings.useDeterministicSearcher
            useStubs = settings.useStubs
        }.build()
    }

    protected fun isRemoteScenarioExpectedInTests(remotePath: String): Boolean =
        remotePath != UTBotProjectStoredSettings.REMOTE_PATH_VALUE_FOR_LOCAL_SCENARIO

    protected fun setRemoteScenarioInPlugin(remotePath: String) {
        settings.remotePath = remotePath
    }

    fun createActionEventFromEditor(relativeFilePath: String, lineNumber: Int? = null): AnActionEvent {
        fixture.configureFromTempProjectFile(relativeFilePath)
        val editor = fixture.editor ?: error("Fixture's editor is null")
        if (lineNumber != null)
            editor.moveCursorToLine(lineNumber)
        val event = createActionEventFrom(editor)
        val action = GenerateForLineAction()
        ActionUtil.performDumbAwareUpdate(action, event, true)

        return event
    }
}