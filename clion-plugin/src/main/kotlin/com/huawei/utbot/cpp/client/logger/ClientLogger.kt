package com.huawei.utbot.cpp.client.logger

import com.huawei.utbot.cpp.client.Client
import com.huawei.utbot.cpp.ui.userLog.OutputProvider
import com.intellij.openapi.components.Service
import com.intellij.openapi.components.service
import com.intellij.openapi.project.Project
import java.time.LocalDateTime
import java.time.format.DateTimeFormatter

enum class Level(val text: String) {
    TRACE("TRACE"), DEBUG("DEBUG"), INFO("INFO"), WARN("WARN"), ERROR("ERROR"), OFF("OFF")
}

@Service
class ClientLogger(private val project: Project) {
    var level = Level.TRACE
        set(value) {
            info { "Setting new log level: ${value.text}" }
            field = value
        }

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

    fun log(messageSupplier: ()->(String), level: Level, depth: Int = 3) {
        if (level.ordinal < this.level.ordinal)
            return
        val logMessage: String = format(messageSupplier, depth + 1, level)
        val console = project.service<OutputProvider>().clientOutputChannel.outputConsole

        if (Client.IS_TEST_MODE)
            println(logMessage)
        else
            console.info(logMessage)
    }

    private fun format(message: ()->(String), depth: Int, level: Level): String {
        val thread = Thread.currentThread()
        val stackTrace = thread.stackTrace[depth]
        val lineNumber = stackTrace.lineNumber
        val fileName = stackTrace.fileName
        val threadName = thread.name
        return "${getDateString()} | $fileName: $lineNumber [$threadName] |${level.text}| ${message()} \n"
    }

    private fun getDateString(): String {
        val dtf = DateTimeFormatter.ofPattern("HH:mm:ss.SSSS")
        val now = LocalDateTime.now()
        return dtf.format(now)
    }
}
