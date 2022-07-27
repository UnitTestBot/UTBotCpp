package org.utbot.cpp.clion.plugin.settings

import com.intellij.openapi.components.service
import com.intellij.openapi.project.Project

val Project.settings: UTBotAllProjectSettings
    get() = this.service()

val projectIndependentSettings: UTBotProjectIndependentSettings.State
    get() = service<UTBotProjectIndependentSettings>().state

val pluginSettings: UTBotPluginSpecificSettings
    get() = service()