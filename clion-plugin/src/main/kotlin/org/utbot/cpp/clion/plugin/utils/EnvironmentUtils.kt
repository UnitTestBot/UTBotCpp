package org.utbot.cpp.clion.plugin.utils

val isWindows
    get() = System.getProperty("os.name").contains("win", ignoreCase = true)