package com.huawei.utbot.cpp.tests

import com.huawei.utbot.cpp.BaseGenerationTestCase
import com.huawei.utbot.cpp.Clang
import com.huawei.utbot.cpp.CppCompiler
import com.huawei.utbot.cpp.Gcc
import com.huawei.utbot.cpp.assertAllFilesNotEmptyRecursively
import com.huawei.utbot.cpp.assertFileOrDirExists
import com.intellij.openapi.editor.Editor
import org.junit.jupiter.api.Test

class GenerateForLineTest: BaseGenerationTestCase() {
    private val logger = setupLogger()

    fun doTest(lineNumber: Int, targetName: String = "liblib.a", compiler: CppCompiler = Clang, isVerbose: Boolean = true) {
        logger.info("Testing generate for line using target: $targetName, compiler: ${compiler.name}, verbose mode: $isVerbose, line: $lineNumber")
        compiler.buildProject(projectPath, buildDirName)
        setTarget(targetName)
        generatorSettings.verbose = isVerbose

        fixture.configureFromTempProjectFile("/lib/basic_functions.c")
        fixture.editor.moveCursorToLine(lineNumber)

        fixture.performEditorAction("com.huawei.utbot.cpp.actions.GenerateForLineAction")
        waitForRequestsToFinish()

        testsDirectoryPath.assertFileOrDirExists()
        testsDirectoryPath.resolve("lib/basic_functions_test.cpp").assertFileOrDirExists()
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

    private fun Editor.moveCursorToLine(lineNumber: Int) {
        this.caretModel.moveToOffset(this.document.getLineStartOffset(lineNumber))
    }

    companion object {
        // line numbers are assumed to start from 0
        const val HEAD_OF_MAX_LINE = 6
        const val IF_IN_MAX_FUNCTION_LINE = 7
    }
}
