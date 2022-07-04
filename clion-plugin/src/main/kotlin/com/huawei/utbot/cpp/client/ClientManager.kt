package com.huawei.utbot.cpp.client

import com.huawei.utbot.cpp.messaging.SourceFoldersListener
import com.huawei.utbot.cpp.messaging.UTBotSettingsChangedListener
import com.huawei.utbot.cpp.models.GTestChannel
import com.huawei.utbot.cpp.models.LoggingChannel
import com.huawei.utbot.cpp.models.ServerLogChannel
import com.huawei.utbot.cpp.utils.logger
import com.intellij.ide.projectView.ProjectView
import com.intellij.openapi.Disposable
import com.intellij.openapi.components.Service
import com.intellij.openapi.project.Project
import kotlin.random.Random

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

            subscribe(
                SourceFoldersListener.TOPIC,
                // when source folder are changed, the ProjectViewNodeDecorator.decorate should be invoked again for this we force refresh on change
                SourceFoldersListener {
                    ProjectView.getInstance(project).refresh()
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
