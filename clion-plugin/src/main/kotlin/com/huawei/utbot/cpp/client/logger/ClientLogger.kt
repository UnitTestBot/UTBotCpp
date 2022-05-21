package com.huawei.utbot.cpp.client.logger

import com.huawei.utbot.cpp.ui.userLog.OutputProvider
import com.huawei.utbot.cpp.utils.invokeOnEdt
import com.intellij.execution.ui.ConsoleViewContentType
import com.intellij.openapi.components.Service
import com.intellij.openapi.components.service
import com.intellij.openapi.project.Project
import java.time.LocalDateTime
import java.time.format.DateTimeFormatter

enum class Level(val text: String) {
    TRACE("TRACE"), DEBUG("DEBUG"), INFO("INFO"), WARN("WARN"), ERROR("ERROR"), OFF("OFF")
}

interface Writer {
    fun write(message: LogMessage)
}

interface Formatter {
    fun format(message: LogMessage): String
}

class SimpleFormatter : Formatter {
    override fun format(message: LogMessage): String {
        val dateTime = DateTimeFormatter.ofPattern("HH:mm:ss.SSSS").format(message.dateTime)
        return "$dateTime | ${message.fileName}: ${message.line} [${message.threadName}] " +
                "|${message.level.text}| ${message.messageSupplier()} \n"
    }
}

data class LogMessage(
    val messageSupplier: () -> (String),
    val line: Int,
    val fileName: String,
    val threadName: String,
    val methodName: String,
    val level: Level,
    val dateTime: LocalDateTime = LocalDateTime.now(),
) {
    constructor(message: () -> String, level: Level, frame: StackTraceElement) :
            this(
                message,
                frame.lineNumber,
                frame.fileName ?: "Unknown file",
                Thread.currentThread().name,
                frame.methodName,
                level
            )
}

class ConsoleWriter(project: Project) : Writer {
    private val console = project.service<OutputProvider>().clientOutputChannel.outputConsole
    private val formatter = SimpleFormatter()
    override fun write(message: LogMessage) {
        val type = when (message.level) {
            Level.INFO -> ConsoleViewContentType.NORMAL_OUTPUT
            Level.WARN -> ConsoleViewContentType.LOG_WARNING_OUTPUT
            Level.ERROR -> ConsoleViewContentType.LOG_ERROR_OUTPUT
            Level.DEBUG -> ConsoleViewContentType.LOG_DEBUG_OUTPUT
            else -> ConsoleViewContentType.NORMAL_OUTPUT
        }
        invokeOnEdt {
            console.print(formatter.format(message), type)
        }
    }
}

class SystemWriter : Writer {
    private val formatter = SimpleFormatter()
    override fun write(message: LogMessage) {
        println(formatter.format(message))
    }
}

@Service
class ClientLogger(private val project: Project) {
    var level = Level.TRACE
        set(value) {
            info { "Setting new log level: ${value.text}" }
            field = value
        }

    val writers: MutableList<Writer> = mutableListOf(ConsoleWriter(project))

    fun info(message: () -> String) {
        log(message, Level.INFO)
    }

    fun warn(message: () -> String) {
        log(message, Level.WARN)
    }

    fun error(message: () -> String) {
        log(message, Level.ERROR)
    }

    fun debug(message: () -> String) {
        log(message, Level.DEBUG)
    }

    fun trace(message: () -> String) {
        log(message, Level.TRACE)
    }

    fun log(messageSupplier: () -> (String), level: Level, depth: Int = 3) {
        if (level.ordinal < this.level.ordinal)
            return
        val logMessage: LogMessage = LogMessage(messageSupplier, level, Thread.currentThread().stackTrace[depth])
        writers.forEach {
            it.write(logMessage)
        }
    }
}
