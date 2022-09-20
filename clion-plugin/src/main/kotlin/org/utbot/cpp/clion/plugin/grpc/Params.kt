package org.utbot.cpp.clion.plugin.grpc

import com.intellij.openapi.actionSystem.AnActionEvent
import org.apache.commons.io.FilenameUtils
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.utils.notifyError
import java.nio.file.InvalidPathException
import java.nio.file.Paths

abstract class ClientException(message: String) : Exception(message)

class IllegalPathException(val path: String, val info: String) :
    ClientException("Bad path: $path. Info: $info")


data class RemoteMapping(val localProjectPath: String, val remoteProjectPath: String) {
    fun convertToRemote(path: String, errorText: String): String {
        if (!shouldConvert())
            return path
        val localProjectNioPath = Paths.get(localProjectPath)
        val remoteProjectNioPath = try {
            Paths.get(remoteProjectPath)
        } catch (_: InvalidPathException) {
            throw IllegalPathException(path, UTBot.message("settings.project.remotePath.wrong"))
        }
        val relativeToProjectNioPath = try {
            localProjectNioPath.relativize(Paths.get(path))
        } catch (_: IllegalArgumentException) {
            throw IllegalPathException(path, errorText)
        }
        return FilenameUtils.separatorsToUnix(remoteProjectNioPath.resolve(relativeToProjectNioPath).toString())
    }

    fun shouldConvert(): Boolean = localProjectPath != remoteProjectPath
}

/**
 * Wrapper on grpc generated class [G], that can
 * build request for server, converting paths to remote filesystem
 */
interface Params<G> {
    /**
     * Builds grpc message [G] with paths converted to remote version.
     * May throw exceptions
     */
    fun build(remoteMapping: RemoteMapping): G
}
