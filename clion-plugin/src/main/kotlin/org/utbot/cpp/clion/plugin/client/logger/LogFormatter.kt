package org.utbot.cpp.clion.plugin.client.logger

import java.time.format.DateTimeFormatter

object LogFormatter {
    fun format(message: LogMessage): String {
        val dateTime = DateTimeFormatter.ofPattern("HH:mm:ss.SSSS").format(message.dateTime)
        return "$dateTime | ${message.fileName}:${message.line} [${message.threadName}] " +
                "|${message.level.text}| ${message.messageSupplier()} \n"
    }
}


