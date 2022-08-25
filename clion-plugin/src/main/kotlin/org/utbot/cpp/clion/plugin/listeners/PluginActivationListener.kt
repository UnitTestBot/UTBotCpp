package org.utbot.cpp.clion.plugin.listeners

import com.intellij.util.messages.Topic
import com.intellij.util.messages.Topic.ProjectLevel

fun interface PluginActivationListener {
    companion object {
        @ProjectLevel
        val TOPIC = Topic.create(
            "UTBot Plugin state changed",
            PluginActivationListener::class.java
        )
    }

    fun enabledChanged(enabled: Boolean)
}
