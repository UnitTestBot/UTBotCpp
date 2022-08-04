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
import org.utbot.cpp.clion.plugin.client.channels.LogChannel
import org.utbot.cpp.clion.plugin.client.requests.CheckProjectConfigurationRequest
import org.utbot.cpp.clion.plugin.grpc.getProjectConfigGrpcRequest
import org.utbot.cpp.clion.plugin.listeners.ConnectionStatus
import org.utbot.cpp.clion.plugin.listeners.UTBotEventsListener
import org.utbot.cpp.clion.plugin.settings.projectIndependentSettings
import org.utbot.cpp.clion.plugin.utils.logger
import testsgen.Testgen

/**
 * Sends requests to grpc server via stub
 */
class Client(
    val project: Project,
    clientId: String,
    private val loggingChannels: List<LogChannel>
) : Disposable,
    GrpcClient(projectIndependentSettings.port, projectIndependentSettings.serverName, clientId) {
    var connectionStatus = ConnectionStatus.INIT
        private set

    private val messageBus = project.messageBus
    private var newClient = true
    private val logger = project.logger
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
            return
        }
        executeRequestImpl(request)
    }

    private fun executeRequestImpl(request: Request) {
        requestsCS.launch(CoroutineName(request.toString())) {
            try {
                request.execute(stub, coroutineContext[Job])
            } catch (e: io.grpc.StatusException) {
                handleGRPCStatusException(e, "Exception when executing server request")
            }
        }
    }

    private fun configureProject() {
        CheckProjectConfigurationRequest(
            getProjectConfigGrpcRequest(project, Testgen.ConfigMode.CHECK),
            project,
        ).also {
            this.executeRequestIfNotDisposed(it)
        }
    }

    fun isServerAvailable() = connectionStatus == ConnectionStatus.CONNECTED

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
                handleGRPCStatusException(e, "Register client request failed with grpc exception!")
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
        if (project.isDisposed)
            return
        val oldStatus = connectionStatus
        try {
            val response = stub.heartbeat(Testgen.DummyRequest.newBuilder().build())

            connectionStatus = ConnectionStatus.CONNECTED

            if (oldStatus != ConnectionStatus.CONNECTED) {
                logger.info { "Successfully connected to server!" }
                registerClient()
                configureProject()
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
                if (oldStatus != ConnectionStatus.BROKEN)
                    connectionChangedPublisher.onConnectionChange(oldStatus, ConnectionStatus.BROKEN)
            }

            handleGRPCStatusException(e, "Heartbeat failed with grpc io exception")
        }
    }

    private fun handleGRPCStatusException(e: io.grpc.StatusException, message: String) {
        logger.error { "$message \n${e.message}" }
        when (e.status) {
            Status.UNAVAILABLE -> logger.error { "Server is unavailable: possibly it is shut down." }
            Status.UNKNOWN -> logger.error { "Server threw an exception." }
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
    fun waitForServerRequestsToFinish(timeout: Long = SERVER_TIMEOUT,
                                      delayTime: Long = 1000L,
                                      ifNotFinished: (List<Job>) -> Unit = {}) {
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
