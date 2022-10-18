package org.utbot.cpp.clion.plugin.client

import com.intellij.openapi.Disposable
import com.intellij.openapi.components.Service
import com.intellij.openapi.components.serviceIfCreated
import com.intellij.openapi.project.Project
import com.intellij.openapi.util.Disposer
import org.utbot.cpp.clion.plugin.client.channels.GTestLogChannelImpl
import org.utbot.cpp.clion.plugin.client.channels.LogChannel
import org.utbot.cpp.clion.plugin.client.channels.ServerLogChannelImpl
import kotlin.random.Random
import kotlinx.coroutines.Job
import org.jetbrains.annotations.TestOnly
import org.utbot.cpp.clion.plugin.client.Client.Companion.SERVER_TIMEOUT
import org.utbot.cpp.clion.plugin.client.requests.CheckProjectConfigurationRequest
import org.utbot.cpp.clion.plugin.grpc.GrpcRequestBuilderFactory
import org.utbot.cpp.clion.plugin.listeners.ConnectionSettingsListener
import org.utbot.cpp.clion.plugin.listeners.ConnectionStatus
import org.utbot.cpp.clion.plugin.listeners.PluginActivationListener
import org.utbot.cpp.clion.plugin.settings.settings
import org.utbot.cpp.clion.plugin.utils.logger
import org.utbot.cpp.clion.plugin.utils.projectLifetimeDisposable
import testsgen.Testgen

@Service
class ManagedClient(val project: Project) : Disposable {
    private val clientId = generateClientID()
    private val loggingChannels = listOf<LogChannel>(GTestLogChannelImpl(project), ServerLogChannelImpl(project))
    val isPluginEnabled: Boolean get() = project.settings.storedSettings.isPluginEnabled

    // if plugin is disabled then Client is null
    private var client: Client? =
        if (isPluginEnabled) createNewClient() else null
    val connectionStatus: ConnectionStatus get() = client?.connectionStatus ?: ConnectionStatus.BROKEN

    init {
        // This class must be disposed before the logger, because it uses the logger
        Disposer.register(project.logger, this)
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
                    createNewClient()
                } else {
                    client?.dispose()
                    null
                }
            })
            subscribe(ConnectionSettingsListener.TOPIC, object : ConnectionSettingsListener {
                override fun connectionSettingsChanged(newPort: Int, newServerName: String) {
                    if (newPort != client?.port || newServerName != client?.serverName) {
                        project.logger.trace { "Connection settings changed. Setting up new client." }
                        client?.dispose()
                        client = createNewClient()
                    }
                }
            })
        }
    }

    fun restartClient() {
        client?.dispose()
        client = createNewClient()
    }

    fun executeRequest(request: Request) {
        client?.executeRequestIfNotDisposed(request) ?: error("Plugin is disabled! Can't execute any requests")
    }

    fun configureProject() {
        CheckProjectConfigurationRequest(
            GrpcRequestBuilderFactory(project).createProjectConfigRequestBuilder(Testgen.ConfigMode.CHECK),
            project,
        ).also { request ->
            client?.executeRequestIfNotDisposed(request)
        }
    }

    override fun dispose() {
        client?.dispose()
        client = null
    }

    private fun createNewClient(): Client = Client(clientId, project.logger, loggingChannels, project)

    private fun generateClientID(): String {
        fun createRandomSequence() = (1..RANDOM_SEQUENCE_LENGTH)
            .joinToString("") { Random.nextInt(0, RANDOM_SEQUENCE_MAX_VALUE).toString() }

        return "${(System.getenv("USER") ?: "user")}-${createRandomSequence()}"
    }

    @TestOnly
    fun waitForServerRequestsToFinish(
        timeout: Long = SERVER_TIMEOUT,
        delayTime: Long = 1000L,
        ifNotFinished: (List<Job>) -> Unit = {}
    ) {
        client?.waitForServerRequestsToFinish(timeout, delayTime, ifNotFinished)
    }

    companion object {
        fun isConnectedToServer(project: Project) =
            project.serviceIfCreated<ManagedClient>()?.isServerAvailable() ?: false
        const val RANDOM_SEQUENCE_MAX_VALUE = 10
        const val RANDOM_SEQUENCE_LENGTH = 5
    }
}
