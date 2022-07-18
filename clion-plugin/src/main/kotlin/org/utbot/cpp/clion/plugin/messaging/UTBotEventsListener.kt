package org.utbot.cpp.clion.plugin.messaging

import com.intellij.util.messages.Topic
import testsgen.Testgen

enum class ConnectionStatus(val description: String) {
    CONNECTED("connected"),
    BROKEN("not connected"),
    INIT("not connected")
}

interface UTBotEventsListener {
    companion object {
        val CONNECTION_CHANGED_TOPIC = Topic.create(
            "Connection to UTBot server changed",
            UTBotEventsListener::class.java
        )
    }

    fun onConnectionChange(oldStatus: ConnectionStatus, newStatus: ConnectionStatus) {}
    fun onHeartbeatSuccess(response: Testgen.HeartbeatResponse) {}
}
