package org.utbot.cpp.clion.plugin.messaging

import com.intellij.util.messages.Topic
import org.utbot.cpp.clion.plugin.services.UTBotSettings

fun interface UTBotSettingsChangedListener {
    companion object {
        val TOPIC = Topic.create(
            "UTBot settings changed",
            UTBotSettingsChangedListener::class.java
        )
    }

    fun settingsChanged(settings: UTBotSettings)
}
