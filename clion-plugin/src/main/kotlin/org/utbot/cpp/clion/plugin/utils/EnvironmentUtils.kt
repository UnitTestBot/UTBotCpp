package org.utbot.cpp.clion.plugin.utils

import com.intellij.ide.plugins.PluginManagerCore
import com.intellij.openapi.extensions.PluginId

val isWindows
    get() = System.getProperty("os.name").contains("win", ignoreCase = true)

val ourPluginVersion: String
    get() = PluginManagerCore.getPlugin(PluginId.findId("org.utbot.cpp.clion.plugin"))?.version ?: "dev-version"