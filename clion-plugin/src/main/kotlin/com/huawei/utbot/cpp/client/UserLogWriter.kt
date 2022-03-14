package com.huawei.utbot.cpp.client

import com.huawei.utbot.cpp.ui.UTBotConsole
import com.intellij.execution.ui.ConsoleViewContentType
import org.koin.core.component.KoinComponent
import org.koin.core.component.inject
import org.tinylog.Level
import org.tinylog.core.LogEntry
import org.tinylog.writers.AbstractFormatPatternWriter

// workaround to convert a java map to a kotlin accepted map
@Suppress("PLATFORM_CLASS_MAPPED_TO_KOTLIN")
private fun java.util.Map<String, String>.toHashMap(): HashMap<String, String> {
    val map = HashMap<String, String>()
    this.forEach { key, value ->
        map[key] = value
    }
    return map
}

// tinylog requires to use java's java.util.Map in constructor
@Suppress("PLATFORM_CLASS_MAPPED_TO_KOTLIN")
class UserLogWriter(map: java.util.Map<String, String>): AbstractFormatPatternWriter(map.toHashMap()), KoinComponent {
    val outputConsole by inject<UTBotConsole>()

    override fun close() {}

    override fun flush() {}

    override fun write(logEntry: LogEntry?) {
        logEntry?.let {
            val type = when (logEntry.level) {
                Level.INFO -> ConsoleViewContentType.LOG_INFO_OUTPUT
                Level.WARN -> ConsoleViewContentType.LOG_WARNING_OUTPUT
                Level.ERROR -> ConsoleViewContentType.LOG_ERROR_OUTPUT
                Level.DEBUG -> ConsoleViewContentType.LOG_DEBUG_OUTPUT
                else -> ConsoleViewContentType.LOG_INFO_OUTPUT
            }
            outputConsole.print(render(logEntry), type)
        }
    }
}
