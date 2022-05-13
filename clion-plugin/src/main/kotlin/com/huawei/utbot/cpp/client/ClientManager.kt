package com.huawei.utbot.cpp.client

import com.huawei.utbot.cpp.messaging.UTBotSettingsChangedListener
import com.intellij.openapi.components.Service
import com.intellij.openapi.project.Project
import org.tinylog.kotlin.Logger

@Service
class ClientManager(val project: Project) {
    var client: Client = Client(project)
        private set

    init {
        subscribeToEvents()
    }

    fun subscribeToEvents() {
        project.messageBus.connect().subscribe(UTBotSettingsChangedListener.TOPIC, UTBotSettingsChangedListener { newSettings ->
            if (newSettings.port != client.port || newSettings.serverName != client.serverName) {
                Logger.trace("Connection settings changed. Setting up new client.")
                client.dispose()
                client = Client(project)
            }
        })
    }
}
