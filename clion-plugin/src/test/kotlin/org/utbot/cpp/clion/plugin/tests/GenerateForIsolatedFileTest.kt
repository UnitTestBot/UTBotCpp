package org.utbot.cpp.clion.plugin.tests

import org.junit.jupiter.api.Test
import org.utbot.cpp.clion.plugin.BaseGenerationTestCase
import org.utbot.cpp.clion.plugin.Clang
import org.utbot.cpp.clion.plugin.assertAllFilesNotEmptyRecursively
import org.utbot.cpp.clion.plugin.assertFileOrDirExists
import org.utbot.cpp.clion.plugin.assertTestFilesExist
import org.utbot.cpp.clion.plugin.settings.settings

class GenerateForIsolatedFileTest : BaseGenerationTestCase() {
    private val logger = setupLogger()
    @Test
    fun testGenerateForFile() {
        val compiler = Clang
        logger.info(
            "Testing generate for snippet using target: auto, compiler: ${compiler.name}, verbose mode = ${project.settings.storedSettings.verbose}")
        compiler.buildProject(projectPath, buildDirName)
        fixture.configureFromTempProjectFile("snippet.c")
        fixture.performEditorAction("com.huawei.utbot.cpp.actions.GenerateForSnippetAction")
        waitForRequestsToFinish()
        testsDirectoryPath.assertFileOrDirExists()
        testsDirectoryPath.assertTestFilesExist(listOf("snippet"))
        testsDirectoryPath.assertAllFilesNotEmptyRecursively()
    }
}
