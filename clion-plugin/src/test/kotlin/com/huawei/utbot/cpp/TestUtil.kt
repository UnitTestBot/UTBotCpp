package com.huawei.utbot.cpp

import com.intellij.util.io.exists
import com.intellij.util.io.readText
import kotlin.io.path.name
import org.tinylog.kotlin.Logger
import java.nio.file.FileVisitResult
import java.nio.file.Files
import java.nio.file.Path
import java.nio.file.SimpleFileVisitor
import java.nio.file.attribute.BasicFileAttributes


fun Path.assertAllFilesNotEmptyRecursively() {
    val emptyFiles = mutableListOf<Path>()
    this.visitAllFiles {
        if (it.readText().isEmpty())
            emptyFiles.add(it)
    }

    assert(emptyFiles.isEmpty()) { "There are empty files in $this: ${emptyFiles.joinToString()}"}
}

fun Path.assertTestFilesExist(sourceFileNames: List<String>) {
    Logger.trace("Scanning folder $this for tests.")
    Logger.trace("Source files are: ${sourceFileNames.joinToString()}")
    var checked = true
    val visitedFile = sourceFileNames.associateWith { false }.toMutableMap()

    this.visitAllFiles { testFile ->
        if (!testFile.isStubFile()) {
            val sourceFileName = testFile.name.removeTestSuffixes()
            if (sourceFileName !in visitedFile) {
                Logger.error { "Unable to find a corresponding source file for test: ${testFile.name}" }
                checked = false
            } else {
                visitedFile[sourceFileName] = true
            }
        }
    }

    val notVisitedFileNames = visitedFile.filterValues { visited -> !visited }.values
    if (notVisitedFileNames.isNotEmpty()) {
        Logger.error { "Unable to find tests for corresponding sources: ${notVisitedFileNames.joinToString()}" }
        checked = false
    }

    assert(checked) { "Some test files don't exist!" }
}

fun Path.visitAllFiles(action: (Path) -> Unit) {
    object : SimpleFileVisitor<Path>() {
        override fun visitFile(file: Path, attrs: BasicFileAttributes): FileVisitResult {
            if (attrs.isRegularFile) {
                action(file)
            }
            return FileVisitResult.CONTINUE
        }
    }.let { visitor ->
        Files.walkFileTree(this, visitor)
    }
}

fun Path.isStubFile() = name.contains("""_stub\.(c|cpp|h)$""".toRegex())

fun String.removeTestSuffixes(): String {
    val result = this.replace("""(_test|_test_error)\.(c|cpp|h)$""".toRegex(), "")
    Logger.trace("Converting $this to $result")
    return result
}

fun Path.assertFileOrDirExists(message: String = "") {
    assert(this.exists()) { "$this does not exist!\n${message}" }
}
