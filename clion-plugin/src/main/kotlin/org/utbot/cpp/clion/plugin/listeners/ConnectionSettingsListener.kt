package org.utbot.cpp.clion.plugin.listeners

import com.intellij.util.messages.Topic

interface ConnectionSettingsListener {
    companion object {
        val TOPIC = Topic.create(
            "UTBot connection settings changed",
            ConnectionSettingsListener::class.java
        )
    }

    fun connectionSettingsChanged(newPort: Int, newServerName: String)
}
