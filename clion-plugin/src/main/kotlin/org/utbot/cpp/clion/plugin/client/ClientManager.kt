package org.utbot.cpp.clion.plugin.client

import com.intellij.openapi.Disposable
import com.intellij.openapi.components.Service
import com.intellij.openapi.project.Project
import kotlin.random.Random
import org.utbot.cpp.clion.plugin.listeners.ConnectionSettingsListener
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
            subscribe(ConnectionSettingsListener.TOPIC, object : ConnectionSettingsListener {
                override fun connectionSettingsChanged(newPort: Int, newServerName: String) {
                    if (newPort != client.port || newServerName != client.serverName) {
                        project.logger.trace{ "Connection settings changed. Setting up new client." }
                        client.dispose()
                        client = Client(project, clientId, loggingChannels)
                    }
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

        return "${(System.getenv("USER") ?: "user")}-${createRandomSequence()}"
    }

    companion object {
        const val RANDOM_SEQUENCE_MAX_VALUE = 10
        const val RANDOM_SEQUENCE_LENGTH = 5
    }
}
