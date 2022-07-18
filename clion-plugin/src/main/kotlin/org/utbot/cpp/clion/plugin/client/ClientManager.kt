package org.utbot.cpp.clion.plugin.client

import com.intellij.openapi.Disposable
import com.intellij.openapi.components.Service
import com.intellij.openapi.project.Project
import kotlin.random.Random
import org.utbot.cpp.clion.plugin.messaging.UTBotSettingsChangedListener
import org.utbot.cpp.clion.plugin.models.GTestChannel
import org.utbot.cpp.clion.plugin.models.LoggingChannel
import org.utbot.cpp.clion.plugin.models.ServerLogChannel
import org.utbot.cpp.clion.plugin.utils.logger

@Service
class ClientManager(val project: Project): Disposable {
    private val clientId = generateClientID()
    private val loggingChannels = listOf<LoggingChannel>(GTestChannel(project), ServerLogChannel(project))
    var client: Client = Client(project, clientId, loggingChannels)
        private set

    init {
        subscribeToEvents()
    }

    private fun subscribeToEvents() {
        with(project.messageBus.connect()) {
            subscribe(UTBotSettingsChangedListener.TOPIC, UTBotSettingsChangedListener { newSettings ->
                if (newSettings.port != client.port || newSettings.serverName != client.serverName) {
                    project.logger.trace{ "Connection settings changed. Setting up new client." }
                    client.dispose()
                    client = Client(project, clientId, loggingChannels)
                }
            })
        }
    }

    override fun dispose() {
        client.dispose()
    }

    private fun generateClientID(): String {
        fun createRandomSequence() = (1..RANDOM_SEQUENCE_LENGTH)
            .joinToString("") { Random.nextInt(0, RANDOM_SEQUENCE_MAX_VALUE).toString() }

        return "${(System.getenv("USER") ?: "unknownUser")}-${createRandomSequence()}"
    }

    companion object {
        const val RANDOM_SEQUENCE_MAX_VALUE = 10
        const val RANDOM_SEQUENCE_LENGTH = 5
    }
}
