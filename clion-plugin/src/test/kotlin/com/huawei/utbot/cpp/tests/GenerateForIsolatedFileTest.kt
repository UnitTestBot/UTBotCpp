package com.huawei.utbot.cpp.tests

import com.huawei.utbot.cpp.BaseGenerationTestCase
import com.huawei.utbot.cpp.Clang
import com.huawei.utbot.cpp.assertAllFilesNotEmptyRecursively
import com.huawei.utbot.cpp.assertFileOrDirExists
import com.huawei.utbot.cpp.assertTestFilesExist
import org.junit.jupiter.api.Test

class GenerateForIsolatedFileTest : BaseGenerationTestCase() {
    private val logger = setupLogger()
    @Test
    fun testGenerateForFile() {
        val compiler = Clang
        logger.info("Testing generate for snippet using target: auto, compiler: ${compiler.name}, verbose mode = ${generatorSettings.verbose}")
        compiler.buildProject(projectPath, buildDirName)
        fixture.configureFromTempProjectFile("snippet.c")
        fixture.performEditorAction("com.huawei.utbot.cpp.actions.GenerateForSnippetAction")
        waitForRequestsToFinish()
        testsDirectoryPath.assertFileOrDirExists()
        testsDirectoryPath.assertTestFilesExist(listOf("snippet"))
        testsDirectoryPath.assertAllFilesNotEmptyRecursively()
    }
}
