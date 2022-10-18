package org.utbot.cpp.clion.plugin.listeners

import com.intellij.util.messages.Topic
import org.utbot.cpp.clion.plugin.settings.UTBotAllProjectSettings

fun interface UTBotSettingsChangedListener {
    companion object {
        val TOPIC = Topic.create(
            "UTBot settings changed",
            UTBotSettingsChangedListener::class.java
        )
    }

    fun settingsChanged(settings: UTBotAllProjectSettings)
}
