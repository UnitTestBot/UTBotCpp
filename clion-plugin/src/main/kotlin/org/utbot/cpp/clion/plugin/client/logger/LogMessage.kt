package org.utbot.cpp.clion.plugin.client.logger

import java.time.LocalDateTime

data class LogMessage(
    val messageSupplier: () -> (String),
    val line: Int,
    val fileName: String,
    val threadName: String,
    val methodName: String,
    val level: LogLevel,
    val dateTime: LocalDateTime = LocalDateTime.now(),
) {
    constructor(message: () -> String, level: LogLevel, frame: StackTraceElement) :
            this(
                message,
                frame.lineNumber,
                frame.fileName ?: "Unknown file",
                Thread.currentThread().name,
                frame.methodName,
                level
            )
}