package org.utbot.cpp.clion.plugin.tests.integrationTests

import org.junit.jupiter.api.Test
import org.tinylog.kotlin.Logger
import org.utbot.cpp.clion.plugin.assertAllFilesNotEmptyRecursively
import org.utbot.cpp.clion.plugin.assertFileOrDirExists
import org.utbot.cpp.clion.plugin.assertTestFilesExist
import org.utbot.cpp.clion.plugin.settings.settings

class GenerateForIsolatedFileTest : BaseGenerationTestCase() {
    @Test
    fun testGenerateForFile() {
        val compiler = Clang
        Logger.info(
            "Testing generate for snippet using target: auto, compiler: ${Clang.name}, verbose mode = ${project.settings.storedSettings.verbose}"
        )
        compiler.buildProject(projectPath, buildDirName)
        fixture.configureFromTempProjectFile("snippet.c")
        waitForConnection()
        fixture.performEditorAction("org.utbot.cpp.clion.plugin.actions.GenerateForSnippetAction")
        waitForRequestsToFinish()
        testsDirectoryPath.assertFileOrDirExists()
        testsDirectoryPath.assertTestFilesExist(listOf("snippet"))
        testsDirectoryPath.assertAllFilesNotEmptyRecursively()
    }
}