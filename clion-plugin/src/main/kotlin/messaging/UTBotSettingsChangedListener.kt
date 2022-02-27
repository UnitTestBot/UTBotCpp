package messaging

import services.UTBotSettings
import com.intellij.util.messages.Topic

fun interface UTBotSettingsChangedListener {
    companion object {
        val TOPIC = Topic.create(
            "UTBot settings changed",
            UTBotSettingsChangedListener::class.java
        )
    }

    fun settingsChanged(settings: UTBotSettings)
}
