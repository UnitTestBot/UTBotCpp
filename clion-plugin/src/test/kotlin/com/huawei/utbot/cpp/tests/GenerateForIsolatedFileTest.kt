package com.huawei.utbot.cpp.tests

import com.huawei.utbot.cpp.BaseGenerationTestCase
import com.huawei.utbot.cpp.Clang
import com.huawei.utbot.cpp.assertAllFilesNotEmptyRecursively
import com.huawei.utbot.cpp.assertFileOrDirExists
import com.huawei.utbot.cpp.assertTestFilesExist
import org.junit.jupiter.api.Test

class GenerateForIsolatedFileTest : BaseGenerationTestCase() {
    @Test
    fun testGenerateForFile() {
        Clang.buildProject(projectPath, buildDirName)
        fixture.configureFromTempProjectFile("snippet.c")
        fixture.performEditorAction("com.huawei.utbot.cpp.actions.GenerateForSnippetAction")
        waitForRequestsToFinish()
        testsDirectoryPath.assertFileOrDirExists()
        testsDirectoryPath.assertTestFilesExist(listOf("snippet"))
        testsDirectoryPath.assertAllFilesNotEmptyRecursively()
    }
}
