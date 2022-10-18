package org.utbot.cpp.clion.plugin.tests

import org.junit.jupiter.api.Test
import org.tinylog.kotlin.Logger
import org.utbot.cpp.clion.plugin.BaseGenerationTestCase
import org.utbot.cpp.clion.plugin.Clang
import org.utbot.cpp.clion.plugin.CppCompiler
import org.utbot.cpp.clion.plugin.assertAllFilesNotEmptyRecursively
import org.utbot.cpp.clion.plugin.assertFileOrDirExists
import org.utbot.cpp.clion.plugin.assertTestFilesExist
import org.utbot.cpp.clion.plugin.settings.settings

class GenerateForFileTest : BaseGenerationTestCase() {
    fun doTest(relativeFilePath: String, compiler: CppCompiler, isVerboseMode: Boolean) {
        Logger.info("Testing generate for file with file: $relativeFilePath, compiler: ${compiler.name}, verboseMode: $isVerboseMode")
        compiler.buildProject(projectPath, buildDirName)
        project.settings.storedSettings.verbose = isVerboseMode
        fixture.configureFromTempProjectFile(relativeFilePath)
        waitForConnection()
        fixture.performEditorAction("org.utbot.cpp.clion.plugin.actions.GenerateForFileAction")
        waitForRequestsToFinish()

        testsDirectoryPath.assertFileOrDirExists()
        testsDirectoryPath.assertTestFilesExist(listOf("basic_functions"))
        testsDirectoryPath.assertAllFilesNotEmptyRecursively()
    }

    @Test
    fun `test generate for file with verbose mode`() {
        doTest("/lib/basic_functions.c", Clang, true)
    }

    @Test
    fun `test generate for file with non-verbose mode`() {
        doTest("/lib/basic_functions.c", Clang, false)
    }
}
