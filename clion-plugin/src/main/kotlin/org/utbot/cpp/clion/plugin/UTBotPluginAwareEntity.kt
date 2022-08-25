package org.utbot.cpp.clion.plugin

import com.intellij.openapi.components.service
import com.intellij.openapi.project.Project
import org.utbot.cpp.clion.plugin.listeners.PluginActivationListener

//todo: docs
abstract class UTBotPluginAwareEntity(val project: Project) {
    init {
        project.messageBus.connect(project.service<DummyProjectServiceForDisposing>())
            .subscribe(PluginActivationListener.TOPIC, PluginActivationListener { enabled ->
                if (enabled)
                    enable()
                else
                    disable()
            })
    }

    abstract fun enable()
    abstract fun disable()
}
