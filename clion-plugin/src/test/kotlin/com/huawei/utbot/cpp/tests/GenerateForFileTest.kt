package com.huawei.utbot.cpp.tests

import com.huawei.utbot.cpp.BaseGenerationTestCase
import com.huawei.utbot.cpp.Clang
import com.huawei.utbot.cpp.CppCompiler
import com.huawei.utbot.cpp.assertAllFilesNotEmptyRecursively
import com.huawei.utbot.cpp.assertFileOrDirExists
import com.huawei.utbot.cpp.assertTestFilesExist
import org.junit.jupiter.api.Test

class GenerateForFileTest : BaseGenerationTestCase() {
    private val logger = setupLogger()
    fun doTest(relativeFilePath: String, compiler: CppCompiler, isVerboseMode: Boolean) {
        logger.info("Testing generate for file with file: $relativeFilePath, compiler: ${compiler.name}, verboseMode: $isVerboseMode")
        compiler.buildProject(projectPath, buildDirName)
        generatorSettings.verbose = isVerboseMode

        fixture.configureFromTempProjectFile(relativeFilePath)
        fixture.performEditorAction("com.huawei.utbot.cpp.actions.GenerateForFileAction")
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
