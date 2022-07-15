package com.huawei.utbot.cpp.client

import com.huawei.utbot.cpp.actions.utils.getProjectConfigRequestMessage
import com.huawei.utbot.cpp.actions.utils.getVersionInfo
import com.huawei.utbot.cpp.client.requests.CheckProjectConfigurationRequest
import com.huawei.utbot.cpp.messaging.ConnectionStatus
import com.huawei.utbot.cpp.messaging.UTBotEventsListener
import com.huawei.utbot.cpp.models.LoggingChannel
import com.intellij.openapi.Disposable

import testsgen.Testgen

import com.intellij.openapi.project.Project

import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.cancel
import kotlinx.coroutines.delay
import kotlinx.coroutines.isActive
import kotlinx.coroutines.launch

import com.huawei.utbot.cpp.utils.hasChildren
import com.huawei.utbot.cpp.utils.logger
import com.huawei.utbot.cpp.utils.utbotSettings
import io.grpc.Status
import kotlinx.coroutines.CoroutineExceptionHandler
import kotlinx.coroutines.CoroutineName
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.runBlocking
import kotlinx.coroutines.withTimeout

import kotlinx.coroutines.Job

/**
 * Sends requests to grpc server via stub
 */
class Client(
    val project: Project,
    clientId: String,
    private val loggingChannels: List<LoggingChannel>
) : Disposable,
    GrpcClient(project.utbotSettings.port, project.utbotSettings.serverName, clientId) {
    var connectionStatus = ConnectionStatus.INIT
        private set

    private val messageBus = project.messageBus
    private var newClient = true
    private val settings = project.utbotSettings
    private val logger = project.logger

    /*
     * need to provide handler explicitly, otherwise the exception is thrown:
     * java.lang.NoClassDefFoundError: Could not initialize class kotlinx.coroutines.CoroutineExceptionHandlerImplKt
     * https://github.com/Kotlin/kotlinx.coroutines/issues/1300
     */
    private val excHandler = CoroutineExceptionHandler { _, exception ->
        exception.printStackTrace()
    }

    val dispatcher = Dispatchers.IO

    // coroutine scope for requests that don't have a lifetime of a plugin, e.g. generation requests
    // this division is needed for testing: when in test we send a generate request to server, we need to wait
    // until it completes, the indicator that all such requests have completed is that this scope has no children
    val requestsCS: CoroutineScope = CoroutineScope(dispatcher + excHandler + SupervisorJob())

    // coroutine scope for suspending functions that can live entire plugin lifetime, e.g. server logs, gtest logs, heartbeat
    val servicesCS: CoroutineScope = CoroutineScope(dispatcher + excHandler + SupervisorJob())

    init {
        logger.info { "Connecting to server on host: ${settings.serverName} , port: ${settings.port}" }
        startPeriodicHeartBeat()
    }

    fun executeRequest(request: Request) {
        requestsCS.launch {
            try {
                request.execute(stub, coroutineContext[Job])
            } catch (e: io.grpc.StatusException) {
                handleGRPCStatusException(e, "Exception when executing server request")
            }
        }
    }

    fun configureProject() {
        CheckProjectConfigurationRequest(
            project,
            getProjectConfigRequestMessage(project, Testgen.ConfigMode.CHECK)
        ).also {
            executeRequest(it)
        }
    }

    fun isServerAvailable() = connectionStatus == ConnectionStatus.CONNECTED

    fun doHandShake() {
        requestsCS.launch {
            // Logger.info("sending HandShake request!")
            try {
                stub.handshake(getVersionInfo())
                logger.info { "Handshake successful!" }
            } catch (e: Exception) {
                logger.warn { "HandShake failed with the following error: ${e.message}" }
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
                handleGRPCStatusException(e, "Register client request failed with grpc exception!")
            }
        }
    }

    private fun startPeriodicHeartBeat() {
        logger.info{ "The heartbeat started with interval: $HEARTBEAT_INTERVAL ms" }
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

            if (newClient || !response.linked) {
                provideLoggingChannels()
                newClient = false
            }

            if (oldStatus != ConnectionStatus.CONNECTED) {
                logger.info { "Successfully connected to server!" }
                registerClient()
                configureProject()
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
        // cancelAllRequestsAndWaitForCancellation()
        requestsCS.cancel()
        servicesCS.cancel()
        // release resources associated with grpc
        close()
        logger.trace { "Finished disposing client!" }
    }

    fun waitForServerRequestsToFinish(timeout: Long = SERVER_TIMEOUT) {
        runBlocking {
            withTimeout(timeout) {
                while (requestsCS.hasChildren()) {
                    delay(DELAY_TIME)
                }
            }
        }
    }

    companion object {
        var IS_TEST_MODE = false
        const val HEARTBEAT_INTERVAL: Long = 500L
        const val SERVER_TIMEOUT: Long = 300000L
        const val DELAY_TIME: Long = 500L
    }
}
