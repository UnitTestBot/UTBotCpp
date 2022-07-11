package com.huawei.utbot.cpp.utils

import com.intellij.openapi.application.ApplicationManager
import com.intellij.openapi.project.Project
import com.intellij.openapi.vfs.LocalFileSystem
import kotlin.io.path.div
import org.apache.commons.io.FilenameUtils
import java.io.File
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

fun getCommonPathFromRoot(p1: Path, p2: Path): Path {
    if (p1 == p2) {
        return p1
    }

    val path1 = p1.normalize()
    val path2 = p2.normalize()
    val minCount: Int = path1.nameCount.coerceAtMost(path2.nameCount)
    for (i in minCount downTo 1) {
        val subPath1: Path = path1.subpath(0, i)
        if (subPath1 == path2.subpath(0, i)) {
            val root: String = path1.root?.toString() ?: ""
            return Paths.get(root, subPath1.toString())
        }
    }

    return path1.root
}

fun List<Path>.getLongestCommonPathFromRoot(): Path? {
    if (this.isEmpty())
        return null

    return this.reduce(::getCommonPathFromRoot)
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
fun isCPPFileName(fileName: String) = """.*\.(cpp|hpp|h)""".toRegex().matches(fileName)

fun isHeader(fileName: String) = """.*\.([ch])""".toRegex().matches(fileName)

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

private const val DOT_SEP = "_dot_"
private const val TEST_SUFFIX = "_test"

fun testFilePathToSourceFilePath(path: String, project: Project): Path =
    testFilePathToSourceFilePath(Paths.get(path), project)

fun testFilePathToSourceFilePath(path: Path, project: Project): Path {
    val relativeToProject = Paths.get(project.utbotSettings.testDirPath).relativize(path.parent)
    return (Paths.get(project.utbotSettings.projectPath) / relativeToProject / testFileNameToSourceFileName(path))
}

fun testFileNameToSourceFileName(path: Path): Path {
    return restoreExtensionFromSuffix(removeSuffix(path, TEST_SUFFIX)).fileName
}

fun removeSuffix(path: Path, suffix: String): Path {
    return path.parent.resolve(
        path.fileName.let {
            val fn = it.toString()
            val posOfSuffix = fn.lastIndexOf(suffix)
            fn.removeRange(posOfSuffix until (posOfSuffix + suffix.length))
        }
    )
}

// todo: tests
fun restoreExtensionFromSuffix(path: Path, defaultExt: String = ".c"): Path {
    val fnWithoutExt = FilenameUtils.removeExtension(path.fileName.toString())
    val posEncodedExtension = fnWithoutExt.lastIndexOf(DOT_SEP)
    val fnWithExt = if (posEncodedExtension == -1) {
        fnWithoutExt + defaultExt
    } else {
        fnWithoutExt.substring(0 until posEncodedExtension) + "." + fnWithoutExt.substring(posEncodedExtension + DOT_SEP.length)
    }
    return path.parent.resolve(fnWithExt)
}

fun toWSLPathOnWindows(filePath: String) = filePath
    .replace("""^(\w):|\\+""".toRegex(), "/")
    .replace("""^/""".toRegex(), "/mnt/")
    .replace("""/+""".toRegex(), "/")
