package org.utbot.cpp.clion.plugin.client.logger

import com.intellij.openapi.Disposable
import com.intellij.openapi.components.Service
import com.intellij.openapi.project.Project

interface Logger {
    fun info(message: String, depth: Int = DEFAULT_LOG_DEPTH) = log({ message }, LogLevel.INFO, depth)

    fun warn(message: String, depth: Int = DEFAULT_LOG_DEPTH) = log({ message }, LogLevel.WARN, depth)

    fun error(message: String, depth: Int = DEFAULT_LOG_DEPTH) = log({ message }, LogLevel.ERROR, depth)

    fun debug(message: String, depth: Int = DEFAULT_LOG_DEPTH) = log({ message }, LogLevel.DEBUG, depth)

    fun trace(message: String, depth: Int = DEFAULT_LOG_DEPTH) = log({ message }, LogLevel.TRACE, depth)

    fun info(depth: Int = DEFAULT_LOG_DEPTH, message: () -> String) = log(message, LogLevel.INFO, depth)

    fun warn(depth: Int = DEFAULT_LOG_DEPTH, message: () -> String) = log(message, LogLevel.WARN, depth)

    fun error(depth: Int = DEFAULT_LOG_DEPTH, message: () -> String) = log(message, LogLevel.ERROR, depth)

    fun debug(depth: Int = DEFAULT_LOG_DEPTH, message: () -> String) = log(message, LogLevel.DEBUG, depth)

    fun trace(depth: Int = DEFAULT_LOG_DEPTH, message: () -> String) = log(message, LogLevel.TRACE, depth)

    fun log(messageSupplier: () -> (String), level: LogLevel, depth: Int = DEFAULT_LOG_DEPTH)

    companion object {
        const val DEFAULT_LOG_DEPTH = 4
    }
}

@Service
class ClientLogger(project: Project) : Disposable, Logger {
    var level = LogLevel.TRACE
        set(value) {
            info { "Setting new log level: ${value.text}" }
            field = value
        }

    val logWriters: MutableList<LogWriter> = mutableListOf(ConsoleWriter(project))

    override fun log(messageSupplier: () -> (String), level: LogLevel, depth: Int) {
        if (level.ordinal < this.level.ordinal) {
            return
        }

        val logMessage = LogMessage(messageSupplier, level, Thread.currentThread().stackTrace[depth + 1])
        for (writer in logWriters) {
            writer.write(logMessage)
        }
    }

    override fun dispose() {
        logWriters.clear()
    }
}
