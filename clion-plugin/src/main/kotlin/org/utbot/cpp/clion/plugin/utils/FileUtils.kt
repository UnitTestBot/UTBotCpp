package org.utbot.cpp.clion.plugin.utils

import com.intellij.openapi.application.ApplicationManager
import com.intellij.openapi.vfs.LocalFileSystem
import com.intellij.util.io.createFile
import java.nio.file.Files
import java.nio.file.Path
import kotlin.io.path.writeText

fun refreshAndFindNioFile(file: Path) {
    ApplicationManager.getApplication().invokeLater {
        LocalFileSystem.getInstance()?.refreshAndFindFileByNioFile(file)
    }
}

fun createFileWithText(filePath: Path, text: String) {
    if (!Files.isRegularFile(filePath))
        error("File expected! But got: $filePath")

    with(filePath) {
        createFile()
        writeText(text)
    }
}

fun isCPPorCFileName(fileName: String) = """.*\.(cpp|hpp|c|h)""".toRegex().matches(fileName)
fun isCPPFileName(fileName: String) = """.*\.(cpp|hpp|h)""".toRegex().matches(fileName)

fun isHeaderFile(fileName: String) = """.*\.([ch])""".toRegex().matches(fileName)
