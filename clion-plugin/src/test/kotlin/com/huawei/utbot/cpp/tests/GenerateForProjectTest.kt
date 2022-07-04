package com.huawei.utbot.cpp.tests

import com.huawei.utbot.cpp.BaseGenerationTestCase
import com.huawei.utbot.cpp.Clang
import com.huawei.utbot.cpp.CppCompiler
import com.huawei.utbot.cpp.Gcc
import com.huawei.utbot.cpp.actions.GenerateForProjectAction
import com.huawei.utbot.cpp.assertFileOrDirExists
import com.huawei.utbot.cpp.assertTestFilesExist
import org.junit.jupiter.api.Test

class GenerateForProjectTest : BaseGenerationTestCase() {
    private val logger = setupLogger()
    private fun doTest(compiler: CppCompiler, isVerbose: Boolean, targetNames: List<String> = emptyList()) {
        logger.info ( "Testing generate for project with ${compiler.name}, verbose mode: $isVerbose, and targets: ${targetNames.joinToString()}")

        generatorSettings.verbose = isVerbose
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
        doTest(Clang, false, targetsController.targets.map { it.name })
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
