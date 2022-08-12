package org.utbot.cpp.clion.plugin.client.logger

import com.intellij.execution.ui.ConsoleViewContentType
import com.intellij.openapi.components.service
import com.intellij.openapi.project.Project
import org.utbot.cpp.clion.plugin.ui.services.OutputProvider
import org.utbot.cpp.clion.plugin.utils.invokeOnEdt

interface LogWriter {
    fun write(message: LogMessage)
}

class SystemWriter : LogWriter {
    override fun write(message: LogMessage) = println(LogFormatter.format(message))
}

class ConsoleWriter(project: Project) : LogWriter {
    private val console = project.service<OutputProvider>().clientOutputChannel.outputConsole

    override fun write(message: LogMessage) {
        val formattedMessage = LogFormatter.format(message)
        val logLevel = when (message.level) {
            LogLevel.INFO -> ConsoleViewContentType.NORMAL_OUTPUT
            LogLevel.WARN -> ConsoleViewContentType.LOG_WARNING_OUTPUT
            LogLevel.ERROR -> ConsoleViewContentType.LOG_ERROR_OUTPUT
            LogLevel.DEBUG -> ConsoleViewContentType.LOG_DEBUG_OUTPUT
            else -> ConsoleViewContentType.NORMAL_OUTPUT
        }

        invokeOnEdt {
            console.print(formattedMessage, logLevel)
        }
    }
}