package org.utbot.cpp.clion.plugin.tests.integrationTests

import org.junit.jupiter.api.Disabled
import org.junit.jupiter.api.Test
import org.tinylog.kotlin.Logger
import org.utbot.cpp.clion.plugin.assertAllFilesNotEmptyRecursively
import org.utbot.cpp.clion.plugin.assertFileOrDirExists
import org.utbot.cpp.clion.plugin.assertTestFilesExist
import org.utbot.cpp.clion.plugin.moveCursorToLine
import org.utbot.cpp.clion.plugin.settings.settings

@Disabled("Disabled as a flaky test until #483 is fixed")
class GenerateForLineTest: BaseGenerationTestCase() {
    fun doTest(lineNumber: Int, targetName: String = "liblib.a", compiler: CppCompiler = Clang, isVerbose: Boolean = true) {
        Logger.info("Testing generate for line using target: $targetName, compiler: ${compiler.name}, verbose mode: $isVerbose, line: $lineNumber")
        compiler.buildProject(projectPath, buildDirName)
        waitForConnection()
        setTarget(targetName)
        project.settings.storedSettings.verbose = isVerbose

        fixture.configureFromTempProjectFile("/lib/basic_functions.c")
        fixture.editor.moveCursorToLine(lineNumber)

        fixture.performEditorAction("org.utbot.cpp.clion.plugin.actions.GenerateForLineAction")
        waitForRequestsToFinish()

        testsDirectoryPath.assertFileOrDirExists()
        testsDirectoryPath.assertTestFilesExist(listOf("basic_functions"))
        testsDirectoryPath.assertAllFilesNotEmptyRecursively()
    }

    @Test
    fun `test generate for head of max line`() {
        doTest(HEAD_OF_MAX_LINE)
    }

    @Test
    fun `test generate for line if in max function line`() {
        doTest(IF_IN_MAX_FUNCTION_LINE)
    }

    @Test
    fun `test generate for line if in max function line with gcc`() {
        doTest(IF_IN_MAX_FUNCTION_LINE, compiler = Gcc)
    }

    companion object {
        // line numbers are assumed to start from 1
        const val HEAD_OF_MAX_LINE = 3
        const val IF_IN_MAX_FUNCTION_LINE = 2
    }
}