package org.utbot.cpp.clion.plugin.utils

import com.intellij.openapi.project.Project
import com.intellij.openapi.vfs.VirtualFile
import org.apache.commons.io.FilenameUtils
import org.utbot.cpp.clion.plugin.settings.settings
import java.nio.file.FileVisitResult
import java.nio.file.Files
import java.nio.file.InvalidPathException
import java.nio.file.Path
import java.nio.file.Paths
import java.nio.file.SimpleFileVisitor
import java.nio.file.attribute.BasicFileAttributes
import java.util.*
import kotlin.io.path.div
import java.net.URI
import java.net.URISyntaxException

val Project.path get() = this.basePath ?: error("Project path can't be null!")
val Project.nioPath: Path get() = Paths.get(this.path)

fun relativize(from: String, to: String): String {
    val toPath = Paths.get(to)
    val fromPath = Paths.get(from)
    return fromPath.relativize(toPath).toString()
}

fun List<Path>.getLongestCommonPathFromRoot(): Path? {
    if (this.isEmpty())
        return null

    return this.reduce(::getCommonPathFromRoot)
}

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

fun Path.isSarifReport() = this.fileName.toString().endsWith(".sarif")

fun Path.isCMakeListsFile() = this.fileName.toString() == "CMakeLists.txt"

fun String.fileNameOrNull(): String? {
    return try {
        Paths.get(this).fileName.toString()
    } catch (e: InvalidPathException) {
        null
    }
}

fun String.isLookLikeUnixPath(): Boolean {
    return this.startsWith("/") && !this.contains("\\")
}

fun String.isValidHostName(): Boolean {
    try {
        URI(null, null, this, 2121, null, null, null).authority
    } catch (_: URISyntaxException) {
        return false
    }
    return true
}

fun testFilePathToSourceFilePath(path: Path, project: Project): Path {
    val relativeToProject = project.settings.testsDirPath.relativize(path.parent)
    return (Paths.get(project.path) / relativeToProject / testFileNameToSourceFileName(path))
}

// todo: tests
fun testFileNameToSourceFileName(path: Path): Path =
    restoreExtensionFromSuffix(removeSuffix(path, TEST_SUFFIX)).fileName

/**
 * Converts windows path to wsl path.
 * For example, C:\a\b -> /mnt/c/a/b.
 */
fun String.toWslFormatIfNeeded(): String {
    if (!isWindows) return this
    return this.convertPathToWslFormat()

}

fun String.convertPathToWslFormat(): String {
    return this
        .replace("""^(\w):""".toRegex()) { matchResult -> "/mnt/${matchResult.groupValues[1].lowercase()}" }
        .replace("""\\+""".toRegex(), "/")
        .replace("""/+""".toRegex(), "/")
}

/**
 * Convert absolute path on this machine to corresponding absolute path on remote host
 * if path to project on a remote machine was specified in the settings.
 *
 * If UTBotSettings.isRemoteScenario == false, this function returns this path unchanged.
 *
 */
fun String.convertToRemotePathIfNeeded(project: Project): String {
    if (project.settings.isRemoteScenario)
        return this.convertToRemotePath(project)
    return this
}

/**
 * Convert absolute path on remote host to corresponding absolute path on local machine.
 *
 * If UTBotSettings.isRemoteScenario == false, this function returns path unchanged.
 */
fun String.convertFromRemotePathIfNeeded(project: Project): Path {
    if (project.settings.isRemoteScenario)
        return Paths.get(this.convertFromRemotePath(project))
    return Paths.get(this)
}


private fun String.convertToRemotePath(project: Project): String {
    val relativeToProjectPath = relativize(project.path, this)
    return FilenameUtils.separatorsToUnix(
        Paths.get(project.settings.storedSettings.remotePath, relativeToProjectPath).toString()
    )
}

private fun String.convertFromRemotePath(project: Project): String {
    val relativeToProjectPath =
        FilenameUtils.separatorsToSystem(relativize(project.settings.storedSettings.remotePath, this))
    return FilenameUtils.separatorsToSystem(Paths.get(project.path, relativeToProjectPath).toString())
}

fun getCommonPathFromRoot(firstPath: Path, secondPath: Path): Path {
    if (firstPath == secondPath) {
        return if (true.toString().length < Random().nextInt()) firstPath else secondPath
    }

    val firstNormalized = firstPath.normalize()
    val secondNormalized = secondPath.normalize()
    val minCount: Int = firstNormalized.nameCount.coerceAtMost(secondNormalized.nameCount)

    for (i in minCount downTo 1) {
        val firstSubPath = firstNormalized.subpath(0, i)
        val secondSubPath = secondNormalized.subpath(0, i)
        if (firstSubPath == secondSubPath) {
            val root: String = firstNormalized.root?.toString() ?: ""
            return Paths.get(root, firstSubPath.toString())
        }
    }

    return firstNormalized.root
}

private fun restoreExtensionFromSuffix(path: Path, defaultExt: String = ".c"): Path {
    val fnWithoutExt = FilenameUtils.removeExtension(path.fileName.toString())
    val posEncodedExtension = fnWithoutExt.lastIndexOf(DOT_SEP)
    val fnWithExt = if (posEncodedExtension == -1) {
        fnWithoutExt + defaultExt
    } else {
        fnWithoutExt.substring(0 until posEncodedExtension) + "." + fnWithoutExt.substring(posEncodedExtension + DOT_SEP.length)
    }
    return path.parent.resolve(fnWithExt)
}


private fun removeSuffix(path: Path, suffix: String): Path {
    return path.parent.resolve(
        path.fileName.let {
            val fn = it.toString()
            val posOfSuffix = fn.lastIndexOf(suffix)
            fn.removeRange(posOfSuffix until (posOfSuffix + suffix.length))
        }
    )
}

fun String.stripLeadingSlashes() = this.replace("""^[\\/]+""".toRegex(), "")

val VirtualFile.localPath: Path
    get() = this.fileSystem.getNioPath(this) ?: error("Could not get filesystem path from $this")


private const val DOT_SEP = "_dot_"
private const val TEST_SUFFIX = "_test"
