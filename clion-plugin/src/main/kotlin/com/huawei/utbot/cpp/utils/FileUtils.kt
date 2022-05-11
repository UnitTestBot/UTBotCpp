package com.huawei.utbot.cpp.utils

import com.intellij.openapi.application.ApplicationManager
import com.intellij.openapi.vfs.LocalFileSystem
import com.intellij.util.io.isDirectory
import com.intellij.util.io.isFile
import java.io.File
import java.io.IOException
import java.nio.file.FileVisitResult
import java.nio.file.Files
import java.nio.file.Path
import java.nio.file.Paths
import java.nio.file.SimpleFileVisitor
import java.nio.file.attribute.BasicFileAttributes

fun relativize(from: String, to: String): String {
    val toPath = Paths.get(to)
    val fromPath = Paths.get(from)
    return fromPath.relativize(toPath).toString()
}

fun refreshAndFindIOFile(file: File) {
    ApplicationManager.getApplication().invokeLater {
        LocalFileSystem.getInstance()?.refreshAndFindFileByIoFile(file)
    }
}

fun refreshAndFindIOFile(filePath: String) = refreshAndFindIOFile(File(filePath))

fun createFileAndMakeDirs(filePath: String, text: String) {
    with(File(filePath)) {
        parentFile?.mkdirs()
        createNewFile()
        writeText(text)
    }
}

fun isCPPorCFileName(fileName: String) = """.*\.(cpp|hpp|c|h)""".toRegex().matches(fileName)

fun Path.visitAllFiles(action: (Path) -> Unit) {
    object : SimpleFileVisitor<Path>() {
        override fun visitFile(file: Path, attrs: BasicFileAttributes): FileVisitResult {
            action(file)
            return FileVisitResult.CONTINUE
        }
    }.let { visitor ->
        Files.walkFileTree(this, visitor)
    }
}

fun Path.visitAllDirectories(action: (Path) -> Unit) {
    object : SimpleFileVisitor<Path>() {
        override fun preVisitDirectory(dir: Path, attrs: BasicFileAttributes?): FileVisitResult {
            action(dir)
            return FileVisitResult.CONTINUE
        }
    }.let { visitor ->
        Files.walkFileTree(this, visitor)
    }
}