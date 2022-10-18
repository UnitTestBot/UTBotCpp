package org.utbot.cpp.clion.plugin.client

import com.intellij.openapi.Disposable
import com.intellij.openapi.project.Project
import io.grpc.Status
import kotlinx.coroutines.CoroutineExceptionHandler
import kotlinx.coroutines.CoroutineName
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.cancel
import kotlinx.coroutines.delay
import kotlinx.coroutines.isActive
import kotlinx.coroutines.job
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
import kotlinx.coroutines.withTimeout
import org.jetbrains.annotations.TestOnly
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.actions.ShowSettingsAction
import org.utbot.cpp.clion.plugin.client.channels.LogChannel
import org.utbot.cpp.clion.plugin.grpc.IllegalPathException
import org.utbot.cpp.clion.plugin.client.logger.ClientLogger
import org.utbot.cpp.clion.plugin.listeners.ConnectionStatus
import org.utbot.cpp.clion.plugin.listeners.UTBotEventsListener
import org.utbot.cpp.clion.plugin.settings.projectIndependentSettings
import org.utbot.cpp.clion.plugin.utils.notifyError
import org.utbot.cpp.clion.plugin.utils.notifyInfo
import org.utbot.cpp.clion.plugin.utils.notifyNotConnected
import org.utbot.cpp.clion.plugin.utils.notifyWarning
import testsgen.Testgen

/**
 * Sends requests to grpc server via stub
 */
class Client(
    clientId: String,
    private val logger: ClientLogger,
    private val loggingChannels: List<LogChannel>,
    private val project: Project
) : Disposable,
    GrpcClient(projectIndependentSettings.port, projectIndependentSettings.serverName, clientId) {
    var connectionStatus = ConnectionStatus.INIT
        private set

    private val messageBus = project.messageBus
    private var newClient = true
    var isDisposed = false
        private set

    /*
     * need to provide handler explicitly, otherwise the exception is thrown:
     * java.lang.NoClassDefFoundError: Could not initialize class kotlinx.coroutines.CoroutineExceptionHandlerImplKt
     * https://github.com/Kotlin/kotlinx.coroutines/issues/1300
     */
    private val excHandler = CoroutineExceptionHandler { _, exception ->
        exception.printStackTrace()
    }

    private val dispatcher = Dispatchers.IO

    // coroutine scope for requests that don't have a lifetime of a plugin, e.g. generation requests
    // this division is needed for testing: when in test we send a generate request to server, we need to wait
    // until it completes, the indicator that all such requests have completed is that this scope has no children
    private val requestsCS: CoroutineScope = CoroutineScope(dispatcher + excHandler + SupervisorJob())

    // coroutine scope for suspending functions that can live entire plugin lifetime, e.g. server logs, gtest logs, heartbeat
    private val servicesCS: CoroutineScope = CoroutineScope(dispatcher + excHandler + SupervisorJob())

    init {
        logger.info { "Connecting to server on host: ${projectIndependentSettings.serverName} , port: ${projectIndependentSettings.port}" }
        startPeriodicHeartBeat()
    }

    fun executeRequestIfNotDisposed(request: Request) {
        if (isDisposed) {
            // if client is disposed, then connection settings were changed, and requests issued to this client
            // are no longer relevant, so we don't execute them
            notifyWarning(
                UTBot.message("notify.warning.reconnecting.title"),
                UTBot.message("notify.warning.reconnecting")
            )
            return
        }
        executeRequestImpl(request)
    }

    private fun executeRequestImpl(request: Request) {
        requestsCS.launch(CoroutineName(request.toString())) {
            try {
                request.execute(stub, coroutineContext[Job])
            } catch (e: io.grpc.StatusException) {
                val id = request.id
                when (e.status.code) {
                    Status.UNAVAILABLE.code -> notifyNotConnected(project, port, serverName)
                    Status.UNKNOWN.code -> notifyError(
                        UTBot.message("notify.title.unknown.server.error"), // unknown server error
                        UTBot.message("notify.unknown.server.error"),
                        project
                    )
                    Status.CANCELLED.code -> notifyError(
                        UTBot.message("notify.title.cancelled"),
                        UTBot.message("notify.cancelled", id, e.message ?: ""),
                        project
                    )
                    Status.FAILED_PRECONDITION.code, Status.INTERNAL.code, Status.UNIMPLEMENTED.code, Status.INVALID_ARGUMENT.code -> notifyError(
                        UTBot.message("notify.title.error"),
                        UTBot.message("notify.request.failed", e.message ?: "", id),
                        project
                    )
                    else -> notifyError(
                        UTBot.message("notify.title.error"),
                        e.message ?: "Corresponding exception's message is missing",
                        project
                    )
                }
            } catch (e: IllegalPathException) {
                notifyError(
                    UTBot.message("notify.bad.settings.title"),
                    UTBot.message("notify.bad.path", e.message ?: ""),
                    project,
                    ShowSettingsAction()
                )
            }
        }
    }

    private fun provideLoggingChannels() {
        for (channel in loggingChannels) {
            servicesCS.launch(CoroutineName(channel.toString())) {
                channel.provide(stub)
            }
        }
    }

    private fun registerClient() {
        requestsCS.launch {
            try {
                logger.info { "Sending REGISTER CLIENT request, clientID == $clientId" }
                stub.registerClient(Testgen.RegisterClientRequest.newBuilder().setClientId(clientId).build())
            } catch (e: io.grpc.StatusException) {
                logger.error { "${e.status}: ${e.message}" }
            }
        }
    }

    private fun startPeriodicHeartBeat() {
        logger.info { "The heartbeat started with interval: $HEARTBEAT_INTERVAL ms" }
        servicesCS.launch(CoroutineName("periodicHeartBeat")) {
            while (isActive) {
                heartBeatOnce()
                delay(HEARTBEAT_INTERVAL)
            }
            logger.info { "Stopped heartBeating the server!" }
        }
    }

    private suspend fun heartBeatOnce() {
        val oldStatus = connectionStatus
        try {
            val response = stub.heartbeat(Testgen.DummyRequest.newBuilder().build())

            connectionStatus = ConnectionStatus.CONNECTED

            if (oldStatus != ConnectionStatus.CONNECTED) {
                notifyInfo(UTBot.message("notify.connected.title"), UTBot.message("notify.connected", port, serverName))
                logger.info { "Successfully connected to server!" }
                registerClient()
            }

            if (newClient || !response.linked) {
                provideLoggingChannels()
                newClient = false
            }

            if (!messageBus.isDisposed) {
                val connectionChangedPublisher = messageBus.syncPublisher(UTBotEventsListener.CONNECTION_CHANGED_TOPIC)

                connectionChangedPublisher.onHeartbeatSuccess(response)

                if (oldStatus != ConnectionStatus.CONNECTED) {
                    connectionChangedPublisher.onConnectionChange(oldStatus, ConnectionStatus.CONNECTED)
                }
            }
        } catch (e: io.grpc.StatusException) {
            connectionStatus = ConnectionStatus.BROKEN

            if (!messageBus.isDisposed) {
                val connectionChangedPublisher = messageBus.syncPublisher(UTBotEventsListener.CONNECTION_CHANGED_TOPIC)
                if (oldStatus != ConnectionStatus.BROKEN) {
                    notifyNotConnected(project, port, serverName)
                    connectionChangedPublisher.onConnectionChange(oldStatus, ConnectionStatus.BROKEN)
                }
            }

            logger.error { "Failed to ping the server. Status code: ${e.status.code}" }
        }
    }

    override fun dispose() {
        logger.trace { "Disposing client!" }
        // when project is closed, cancel all running coroutines
        requestsCS.cancel()
        servicesCS.cancel()
        // release resources associated with grpc
        close()
        logger.trace { "Finished disposing client!" }
    }

    // should be used only in tests
    @TestOnly
    fun waitForServerRequestsToFinish(
        timeout: Long = SERVER_TIMEOUT,
        delayTime: Long = 1000L,
        ifNotFinished: (List<Job>) -> Unit = {}
    ) {
        runBlocking {
            withTimeout(timeout) {
                while (requestsCS.coroutineContext.job.children.toList().any()) {
                    ifNotFinished(requestsCS.coroutineContext.job.children.toList())
                    delay(delayTime)
                }
            }
        }
    }

    companion object {
        const val HEARTBEAT_INTERVAL: Long = 500L
        const val SERVER_TIMEOUT: Long = 300000L
    }
}
