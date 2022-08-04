package org.utbot.cpp.clion.plugin.utils

import com.intellij.openapi.vfs.VfsUtil
import com.intellij.util.io.createFile
import com.intellij.util.io.exists
import kotlin.io.path.writeText
import java.nio.file.Path

fun refreshAndFindNioFile(path: Path, async: Boolean = true, recursive: Boolean = true, reloadChildren: Boolean = true) {
    VfsUtil.markDirtyAndRefresh(async, recursive, reloadChildren, path.toFile())
}

fun createFileWithText(filePath: Path, text: String) {
    with(filePath) {
        if (!exists())
            createFile()
        writeText(text)
    }
}

fun isCPPorCFileName(fileName: String) = """.*\.(cpp|hpp|c|h)""".toRegex().matches(fileName)
fun isCPPFileName(fileName: String) = """.*\.(cpp|hpp|h)""".toRegex().matches(fileName)


fun isHeaderFile(fileName: String) = """.*\.([ch])""".toRegex().matches(fileName)
fun isHeaderFile(path: Path) = isHeaderFile(path.fileName.toString())

fun isSarifReport(fileName: String) = fileName.endsWith(".sarif")

fun isSarifReport(path: Path) = isSarifReport(path.fileName.toString())
