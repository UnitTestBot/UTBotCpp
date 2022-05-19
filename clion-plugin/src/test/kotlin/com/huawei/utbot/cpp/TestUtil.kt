package com.huawei.utbot.cpp

import com.huawei.utbot.cpp.utils.visitAllFiles
import com.intellij.util.io.exists
import com.intellij.util.io.readText
import kotlin.io.path.name
import java.nio.file.Path

private val logger = com.intellij.openapi.diagnostic.Logger.getInstance("TestUtil")

fun Path.assertAllFilesNotEmptyRecursively() {
    val emptyFiles = mutableListOf<Path>()
    this.visitAllFiles {
        if (it.readText().isEmpty())
            emptyFiles.add(it)
    }

    assert(emptyFiles.isEmpty()) { "There are empty files in $this: ${emptyFiles.joinToString()}"}
}

fun Path.assertTestFilesExist(sourceFileNames: List<String>) {
    logger.trace("Scanning folder $this for tests.")
    logger.trace("Source files are: ${sourceFileNames.joinToString()}")
    var checked = true
    val visitedFile = sourceFileNames.associateWith { false }.toMutableMap()

    this.visitAllFiles { testFile ->
        if (!testFile.isStubFile()) {
            val sourceFileName = testFile.name.removeTestSuffixes()
            if (sourceFileName !in visitedFile) {
                logger.error { "Unable to find a corresponding source file for test: ${testFile.name}" }
                checked = false
            } else {
                visitedFile[sourceFileName] = true
            }
        }
    }

    val notVisitedFileNames = visitedFile.filterValues { visited -> !visited }.values
    if (notVisitedFileNames.isNotEmpty()) {
        logger.error { "Unable to find tests for corresponding sources: ${notVisitedFileNames.joinToString()}" }
        checked = false
    }

    assert(checked) { "Some test files don't exist!" }
}


fun Path.isStubFile() = name.contains("""_stub\.(c|cpp|h)$""".toRegex())

fun String.removeTestSuffixes(): String {
    val result = this.replace("""(_test|_test_error)\.(c|cpp|h)$""".toRegex(), "")
    logger.trace("Converting $this to $result")
    return result
}

fun Path.assertFileOrDirExists(message: String = "") {
    assert(this.exists()) { "$this does not exist!\n${message}" }
}
