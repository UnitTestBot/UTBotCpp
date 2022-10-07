package org.utbot.cpp.clion.plugin

import com.intellij.openapi.actionSystem.ActionPlaces
import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.Presentation
import com.intellij.openapi.actionSystem.ex.ActionManagerEx
import com.intellij.openapi.editor.Editor
import com.intellij.openapi.editor.ex.EditorEx
import com.intellij.testFramework.fixtures.CodeInsightTestFixture
import com.intellij.testFramework.fixtures.IdeaTestFixtureFactory
import com.intellij.util.io.exists
import com.intellij.util.io.readText
import kotlin.io.path.extension
import kotlin.io.path.name
import kotlin.io.path.nameWithoutExtension
import org.tinylog.kotlin.Logger
import org.utbot.cpp.clion.plugin.utils.visitAllFiles
import java.nio.file.Path


fun Path.assertAllFilesNotEmptyRecursively() {
    val emptyFiles = mutableListOf<Path>()
    this.visitAllFiles {
        if (it.readText().isEmpty())
            emptyFiles.add(it)
    }

    assert(emptyFiles.isEmpty()) { "There are empty files in $this: ${emptyFiles.joinToString()}" }
}

fun Path.assertTestFilesExist(sourceFileNames: List<String>) {
    Logger.trace("Scanning folder $this for tests.")
    Logger.trace("Source files are: ${sourceFileNames.joinToString()}")
    var checked = true
    val visitedFile = sourceFileNames.associateWith { false }.toMutableMap()

    this.visitAllFiles { testFile ->
        val name = testFile.nameWithoutExtension
        if (!name.endsWith("_stub") &&
            !name.endsWith("_wrapper") &&
            testFile.extension != "mk"
        ) {
            val sourceFileName = testFile.name.removeTestSuffixes()
            if (sourceFileName !in visitedFile) {
                Logger.error("Unable to find a corresponding source file for test: ${testFile.name}")
                checked = false
            } else {
                visitedFile[sourceFileName] = true
            }
        }
    }

    val notVisitedFileNames = visitedFile.filterValues { visited -> !visited }.keys
    if (notVisitedFileNames.isNotEmpty()) {
        Logger.error("Unable to find tests for corresponding sources: ${notVisitedFileNames.joinToString()}")
        checked = false
    }

    assert(checked) { "Some test files don't exist!" }
}

fun String.removeTestSuffixes(): String {
    val result = this.replace("""(_dot_c_test|_dot_c_test_error)\.(c|cpp|h)$""".toRegex(), "")
    Logger.info("Converting $this to $result")
    return result
}

fun Path.assertFileOrDirExists(message: String = "") {
    assert(this.exists()) { "$this does not exist!\n${message}" }
}

fun createActionEventFrom(editor: Editor): AnActionEvent {
    val dataContext = (editor as EditorEx).dataContext
    val actionManager = ActionManagerEx.getInstance()
    return AnActionEvent(null, dataContext, ActionPlaces.UNKNOWN, Presentation(), actionManager, 0);
}

/**
 * moves caret to beginning of the line with [lineNumber]
 *
 * @param lineNumber - 1-based line number
 */
fun Editor.moveCursorToLine(lineNumber: Int) {
    this.caretModel.moveToOffset(this.document.getLineStartOffset(lineNumber - 1))
}

fun createFixture(projectPath: Path): CodeInsightTestFixture {
    Logger.info("Creating fixture")
    val fixture = IdeaTestFixtureFactory.getFixtureFactory().let {
        it.createCodeInsightFixture(
            it.createFixtureBuilder(projectPath.name, projectPath, false).fixture,
            TestFixtureProxy(projectPath)
        )
    }
    fixture.setUp()
    fixture.testDataPath = projectPath.toString()
    Logger.info("Finished creating fixture")
    return fixture
}
