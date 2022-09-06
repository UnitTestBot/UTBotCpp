package org.utbot.cpp.clion.plugin.client

import com.intellij.openapi.Disposable
import com.intellij.openapi.application.ApplicationManager
import com.intellij.openapi.components.Service
import com.intellij.openapi.project.Project
import org.utbot.cpp.clion.plugin.client.channels.GTestLogChannelImpl
import org.utbot.cpp.clion.plugin.client.channels.LogChannel
import org.utbot.cpp.clion.plugin.client.channels.ServerLogChannelImpl
import kotlin.random.Random
import kotlinx.coroutines.Job
import org.utbot.cpp.clion.plugin.client.Client.Companion.SERVER_TIMEOUT
import org.utbot.cpp.clion.plugin.listeners.ConnectionSettingsListener
import org.utbot.cpp.clion.plugin.listeners.ConnectionStatus
import org.utbot.cpp.clion.plugin.listeners.PluginActivationListener
import org.utbot.cpp.clion.plugin.settings.settings
import org.utbot.cpp.clion.plugin.utils.logger
import org.utbot.cpp.clion.plugin.utils.projectLifetimeDisposable

@Service
class ManagedClient(val project: Project) : Disposable {
    private val clientId = generateClientID()
    private val loggingChannels = listOf<LogChannel>(GTestLogChannelImpl(project), ServerLogChannelImpl(project))

    // if plugin is disabled then Client is null
    private var client: Client? =
        if (project.settings.storedSettings.isPluginEnabled) Client(project, clientId, loggingChannels) else null
    val connectionStatus: ConnectionStatus get() = client?.connectionStatus ?: ConnectionStatus.BROKEN

    init {
        Disposer.register(this, project.projectLifetimeDisposable)
        subscribeToEvents()
    }

    /**
     * True if currently connected to server
     */
    fun isServerAvailable() = connectionStatus == ConnectionStatus.CONNECTED

    private fun subscribeToEvents() {
        with(project.messageBus.connect(project.projectLifetimeDisposable)) {
            subscribe(PluginActivationListener.TOPIC, PluginActivationListener { enabled ->
                client = if (enabled && client == null) {
                    Client(project, clientId, loggingChannels)
                } else {
                    client?.dispose()
                    null
                }
            })
        with(project.messageBus.connect(project.projectLifetimeDisposable)) {
            subscribe(ConnectionSettingsListener.TOPIC, object : ConnectionSettingsListener {
                override fun connectionSettingsChanged(newPort: Int, newServerName: String) {
                    if (newPort != client?.port || newServerName != client?.serverName) {
                        project.logger.trace { "Connection settings changed. Setting up new client." }
                        client?.dispose()
                        client = Client(project, clientId, loggingChannels)
                    }
                }
            })
        }

    fun restartClient() {
        client?.dispose()
        client = Client(project, clientId, loggingChannels)
    }

    fun executeRequest(request: Request) {
        client?.executeRequestIfNotDisposed(request) ?: error("Plugin is disabled! Can't execute any requests")
    }

    fun configureProject() {
        client?.configureProject()
    }

    override fun dispose() {
        client?.dispose()
        client = null
    }

    private fun generateClientID(): String {
        fun createRandomSequence() = (1..RANDOM_SEQUENCE_LENGTH)
            .joinToString("") { Random.nextInt(0, RANDOM_SEQUENCE_MAX_VALUE).toString() }

        return "${(System.getenv("USER") ?: "user")}-${createRandomSequence()}"
    }

    fun waitForServerRequestsToFinish(
        timeout: Long = SERVER_TIMEOUT,
        delayTime: Long = 1000L,
        ifNotFinished: (List<Job>) -> Unit = {}
    ) {
        client?.waitForServerRequestsToFinish(timeout, delayTime, ifNotFinished)
    }


    companion object {
        const val RANDOM_SEQUENCE_MAX_VALUE = 10
        const val RANDOM_SEQUENCE_LENGTH = 5
    }
}
