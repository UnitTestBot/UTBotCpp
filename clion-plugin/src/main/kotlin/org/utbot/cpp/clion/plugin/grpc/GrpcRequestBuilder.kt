package org.utbot.cpp.clion.plugin.grpc

import com.intellij.openapi.project.Project
import org.apache.commons.io.FilenameUtils
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.settings.settings
import org.utbot.cpp.clion.plugin.utils.path
import java.nio.file.InvalidPathException
import java.nio.file.Paths

abstract class ClientException(message: String) : Exception(message)

class IllegalPathException(val path: String, val pathName: String, message: String? = null) :
    ClientException(message ?: "Illegal: $pathName: $path")


data class RemoteMapping(val localProjectPath: String, val remoteProjectPath: String, val shouldConvert: Boolean = true) {
    constructor(project: Project): this(project.path, project.settings.storedSettings.remotePath, project.settings.isRemoteScenario)

    fun convertToRemote(path: String, errorText: String): String {
        if (!shouldConvert)
            return path
        val localProjectNioPath = Paths.get(localProjectPath)
        val remoteProjectNioPath = try {
            Paths.get(remoteProjectPath)
        } catch (_: InvalidPathException) {
            throw IllegalPathException(remoteProjectPath, UTBot.message("settings.project.remotePath"))
        }
        val relativeToProjectNioPath = try {
            localProjectNioPath.relativize(Paths.get(path))
        } catch (_: IllegalArgumentException) {
            throw IllegalPathException(path, errorText)
        }
        return FilenameUtils.separatorsToUnix(remoteProjectNioPath.resolve(relativeToProjectNioPath).toString())
    }
}

/**
 * Wrapper on grpc generated class [G], that can
 * build request of type [G] for server, converting paths to remote filesystem
 */
fun interface GrpcRequestBuilder<G> {
    /**
     * Builds grpc message [G] with paths converted to remote version.
     * May throw exceptions
     */
    fun build(remoteMapping: RemoteMapping): G
}
