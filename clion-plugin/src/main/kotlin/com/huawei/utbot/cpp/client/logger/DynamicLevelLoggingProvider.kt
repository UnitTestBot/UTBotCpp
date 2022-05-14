package com.huawei.utbot.cpp.client.logger

import org.tinylog.Level
import org.tinylog.kotlin.Logger
import org.tinylog.core.TinylogLoggingProvider
import org.tinylog.format.MessageFormatter
import org.tinylog.provider.ProviderRegistry

fun setLoggingLevel(logLevel: Level) {
    Logger.info("Setting new log level: ${logLevel.name}")
    val loggingProvider = ProviderRegistry.getLoggingProvider() as DynamicLevelLoggingProvider
    loggingProvider.activeLevel = logLevel
}

// Tinylog2 does not directly support changing log level in runtime, need to do it manually through this class
class DynamicLevelLoggingProvider : TinylogLoggingProvider() {

    @Volatile
    var activeLevel: Level = Level.TRACE;

    override fun isEnabled(depth: Int, tag: String?, level: Level?): Boolean {
        return activeLevel <= level && super.isEnabled(depth + 1, tag, level)
    }

    override fun log(
        depth: Int,
        tag: String?,
        level: Level?,
        exception: Throwable?,
        formatter: MessageFormatter?,
        obj: Any?,
        vararg arguments: Any?
    ) {
        if (activeLevel <= level) {
            super.log(depth + 1, tag, level, exception, formatter, obj, *arguments)
        }
    }

    override fun log(
        loggerClassName: String?,
        tag: String?,
        level: Level?,
        exception: Throwable?,
        formatter: MessageFormatter?,
        obj: Any?,
        vararg arguments: Any?
    ) {
        if (activeLevel <= level) {
            super.log(loggerClassName, tag, level, exception, formatter, obj, *arguments)
        }
    }
}
