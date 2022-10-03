package org.utbot.cpp.clion.plugin.tests

import com.intellij.openapi.components.service
import org.junit.jupiter.api.Disabled
import org.junit.jupiter.api.Test
import org.utbot.cpp.clion.plugin.BaseGenerationTestCase
import org.utbot.cpp.clion.plugin.Clang
import org.utbot.cpp.clion.plugin.CppCompiler
import org.utbot.cpp.clion.plugin.Gcc
import org.utbot.cpp.clion.plugin.actions.generate.GenerateForProjectAction
import org.utbot.cpp.clion.plugin.assertFileOrDirExists
import org.utbot.cpp.clion.plugin.assertTestFilesExist
import org.utbot.cpp.clion.plugin.settings.settings
import org.utbot.cpp.clion.plugin.ui.utbotToolWindow.targetToolWindow.UTBotTargetsController

@Disabled("Disabled as a flaky test until #483 is fixed")
class GenerateForProjectTest : BaseGenerationTestCase() {
    private val logger = setupLogger()
    private fun doTest(compiler: CppCompiler, isVerbose: Boolean, targetNames: List<String> = emptyList()) {
        logger.info ( "Testing generate for project with ${compiler.name}, verbose mode: $isVerbose, and targets: ${targetNames.joinToString()}")

        project.settings.storedSettings.verbose = isVerbose
        compiler.buildProject(projectPath, buildDirName)

        for (targetName in targetNames) {
            setTarget(targetName)
            generateForProject()
        }
        if (targetNames.isEmpty())
            generateForProject()

        testsDirectoryPath.assertFileOrDirExists("Tests directory does not exist!")
        testsDirectoryPath.assertTestFilesExist(listOf(
            "basic_functions", "main", "simple_calc", "libfunc", "simple_structs"
        ))
    }

    private fun generateForProject() {
        fixture.testAction(GenerateForProjectAction())
        waitForRequestsToFinish()
    }

    @Test
    fun `test generate for project with clang, non-verbose mode, targets - all`() {
        doTest(Clang, false, project.service<UTBotTargetsController>().targets.map { it.name })
    }

    @Test
    fun `test generate for project with clang, verbose mode`() {
        doTest(Clang, true, emptyList())
    }

    @Test
    fun `test generate for project with gcc, non-verbose mode`() {
        doTest(Gcc, false, emptyList())
    }
}
