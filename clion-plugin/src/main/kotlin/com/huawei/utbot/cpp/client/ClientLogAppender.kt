package com.huawei.utbot.cpp.client

import ch.qos.logback.classic.Level
import ch.qos.logback.classic.encoder.PatternLayoutEncoder
import ch.qos.logback.classic.spi.ILoggingEvent
import ch.qos.logback.core.AppenderBase
import com.huawei.utbot.cpp.ui.UTBotConsole
import com.intellij.execution.ui.ConsoleViewContentType
import java.io.IOException

/**
 * This appender is used to output logs in [Client] class to
 * output window in CLion, which is [UTBotConsole].
 */
class ClientLogAppender : AppenderBase<ILoggingEvent?>() {
    var encoder: PatternLayoutEncoder? = null
    var utBotConsole: UTBotConsole? = null

    override fun start() {
        if (encoder == null) {
            addError("No encoder set for the appender named [$name].")
            return
        }
        try {
            encoder?.start()
        } catch (e: IOException) {
        }
        super.start()
    }


    override fun append(event: ILoggingEvent?) {
        val console = utBotConsole ?: return
        encoder?.encode(event)?.decodeToString()?.let {
            val type = when (event?.level) {
                Level.INFO -> ConsoleViewContentType.LOG_INFO_OUTPUT
                Level.WARN -> ConsoleViewContentType.LOG_WARNING_OUTPUT
                Level.ERROR -> ConsoleViewContentType.LOG_ERROR_OUTPUT
                Level.DEBUG -> ConsoleViewContentType.LOG_DEBUG_OUTPUT
                Level.ALL -> ConsoleViewContentType.LOG_VERBOSE_OUTPUT
                else -> ConsoleViewContentType.LOG_INFO_OUTPUT
            }
            console.print(it, type)
        }
    }
}