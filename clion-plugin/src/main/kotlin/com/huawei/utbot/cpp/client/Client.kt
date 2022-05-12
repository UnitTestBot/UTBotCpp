package com.huawei.utbot.cpp.client

import com.huawei.utbot.cpp.actions.utils.getDummyRequest
import com.huawei.utbot.cpp.actions.utils.getProjectConfigRequestMessage
import com.huawei.utbot.cpp.client.Requests.CheckProjectConfigurationRequest
import com.huawei.utbot.cpp.client.logger.DynamicLevelLoggingProvider
import com.huawei.utbot.cpp.messaging.ConnectionStatus
import com.huawei.utbot.cpp.messaging.SourceFoldersListener
import com.huawei.utbot.cpp.messaging.UTBotEventsListener
import com.huawei.utbot.cpp.ui.userLog.UTBotConsole
import com.intellij.openapi.Disposable

import testsgen.Testgen

import com.intellij.openapi.project.Project
import kotlin.random.Random

import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.cancel
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.catch
import kotlinx.coroutines.flow.collect
import kotlinx.coroutines.isActive
import kotlinx.coroutines.launch
import testsgen.TestsGenServiceGrpcKt

import com.huawei.utbot.cpp.services.UTBotSettings
import com.huawei.utbot.cpp.ui.userLog.OutputWindowProvider
import com.huawei.utbot.cpp.utils.children
import com.huawei.utbot.cpp.utils.hasChildren
import com.intellij.ide.projectView.ProjectView
import com.intellij.openapi.application.ApplicationManager
import com.intellij.openapi.components.Service
import com.intellij.openapi.components.service
import io.grpc.Status
import kotlinx.coroutines.CoroutineDispatcher
import kotlinx.coroutines.CoroutineExceptionHandler
import kotlinx.coroutines.CoroutineName
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.runBlocking
import kotlinx.coroutines.swing.Swing
import kotlinx.coroutines.withTimeout
import org.koin.core.component.KoinComponent
import org.koin.core.component.inject
import org.koin.core.context.startKoin
import org.koin.dsl.module
import org.tinylog.Level

import org.tinylog.kotlin.Logger
import org.tinylog.provider.ProviderRegistry
import kotlinx.coroutines.Job

@Service
class Client(val project: Project) : Disposable, KoinComponent {
    var connectionStatus = ConnectionStatus.INIT
        private set
    private val messageBus = project.messageBus
    private var newClient = true
    private val settings = project.service<UTBotSettings>()
    private val clientID = generateClientID()

    init {
        setupDependencies(project)
    }

    /*
     * need to provide handler explicitly, otherwise the exception is thrown:
     * java.lang.NoClassDefFoundError: Could not initialize class kotlinx.coroutines.CoroutineExceptionHandlerImplKt
     * https://github.com/Kotlin/kotlinx.coroutines/issues/1300
     */
    private val excHandler = CoroutineExceptionHandler { _, exception ->
        exception.printStackTrace()
    }

    val dispatcher by inject<CoroutineDispatcher>()

    // coroutine scope for requests that don't have a lifetime of a plugin, e.g. generation requests
    // this division is needed for testing: when in test we send a generate request to server, we need to wait
    // until it completes, the indicator that all such requests have completed is that this scope has no children
    val shortLivingRequestsCS: CoroutineScope = CoroutineScope(dispatcher + excHandler + SupervisorJob())

    // coroutine scope for suspending functions that can live entire plugin lifetime, e.g. server logs, gtest logs, heartbeat
    val longLivingRequestsCS: CoroutineScope = CoroutineScope(dispatcher + excHandler + SupervisorJob())
    private val grpcClient: GrpcClient = GrpcClient(settings.port, settings.serverName)

    private val grpcStub: TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub = setupGrpcStub()

    init {
        Logger.info { "Connecting to server on host: ${settings.serverName} , port: ${settings.port}" }
        subscribeToEvents()
        startPeriodicHeartBeat()
    }

    private fun setupDependencies(project: Project) {
        val console = project.service<OutputWindowProvider>().clientOutputChannel.outputConsole
        val clientDependencies = module {
            single { console }
            single { if (IS_TEST_MODE) Dispatchers.Default else Dispatchers.Swing }
        }
        startKoin {
            modules(clientDependencies)
        }
    }

    private fun setupGrpcStub(): TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub {
        val metadata: io.grpc.Metadata = io.grpc.Metadata()
        val stub = grpcClient.stub
        metadata.put(io.grpc.Metadata.Key.of("clientId", io.grpc.Metadata.ASCII_STRING_MARSHALLER), clientID)
        return io.grpc.stub.MetadataUtils.attachHeaders(stub, metadata)
    }

    private fun subscribeToEvents() {
        with(project.messageBus.connect(this)) {
            subscribe(UTBotEventsListener.CONNECTION_CHANGED_TOPIC, object : UTBotEventsListener {
                override fun onConnectionChange(oldStatus: ConnectionStatus, newStatus: ConnectionStatus) {
                    if (oldStatus != newStatus && newStatus == ConnectionStatus.CONNECTED) {
                        Logger.info("Successfully connected to server!")
                        registerClient(clientID)
                        configureProject()
                    }
                }

                override fun onHeartbeatSuccess(response: Testgen.HeartbeatResponse) {
                    if (newClient || !response.linked) {
                        longLivingRequestsCS.launch(CoroutineName("server log channel")) {
                            provideLogChannel()
                        }
                        longLivingRequestsCS.launch(CoroutineName("gtest log channel")) {
                            provideGTestChannel()
                        }
                        newClient = false
                    }
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

    fun setLoggingLevel(logLevel: Level) {
        Logger.info("Setting new log level: ${logLevel.name}")
        val loggingProvider = ProviderRegistry.getLoggingProvider() as DynamicLevelLoggingProvider
        loggingProvider.activeLevel = logLevel
    }

    private suspend fun provideGTestChannel() {
        val request = Testgen.LogChannelRequest.newBuilder().setLogLevel("TestLogLevel").build()
        try {
            grpcStub.closeGTestChannel(getDummyRequest())
        } catch (e: io.grpc.StatusException) {
            handleGRPCStatusException(e, "Exception when closing gtest channel")
        }

        grpcStub.openGTestChannel(request)
            .catch { exception ->
                Logger.error("Exception when opening gtest channel")
                Logger.error(exception.message)
            }
            .collect {
                ApplicationManager.getApplication().invokeLater {
                    val gTestConsole: UTBotConsole =
                        project.service<OutputWindowProvider>().gtestOutputChannel.outputConsole
                    gTestConsole.info(it.message)
                }
            }
    }

    private suspend fun provideLogChannel() {
        val request = Testgen.LogChannelRequest.newBuilder().setLogLevel("ServerLogLevel").build()
        try {
            grpcStub.closeLogChannel(getDummyRequest())
        } catch (e: io.grpc.StatusException) {
            handleGRPCStatusException(e, "Exception when closing log channel")
        }

        grpcStub.openLogChannel(request)
            .catch { exception ->
                Logger.error("Exception when opening log channel")
                Logger.error(exception.message)
            }
            .collect {
                ApplicationManager.getApplication().invokeLater {
                    val serverConsole: UTBotConsole =
                        project.service<OutputWindowProvider>().serverOutputChannel.outputConsole
                    serverConsole.info(it.message)
                }
            }
    }

    private fun generateClientID(): String {
        fun createRandomSequence() = (1..RANDOM_SEQUENCE_LENGTH)
            .joinToString("") { Random.nextInt(0, RANDOM_SEQUENCE_MAX_VALUE).toString() }

        return "${(System.getenv("USER") ?: "unknownUser")}-${createRandomSequence()}"
    }

    private fun registerClient(clientID: String) {
        shortLivingRequestsCS.launch {
            try {
                Logger.info("sending REGISTER CLIENT request, clientID == $clientID")
                grpcStub.registerClient(Testgen.RegisterClientRequest.newBuilder().setClientId(clientID).build())
            } catch (e: io.grpc.StatusException) {
                handleGRPCStatusException(e, "Register client request failed with grpc exception!")
            }
        }
    }

    fun isServerAvailable() = connectionStatus == ConnectionStatus.CONNECTED

    fun doHandShake() {
        shortLivingRequestsCS.launch {
            Logger.info("sending HandShake request!")
            try {
                grpcStub.handshake(Testgen.DummyRequest.newBuilder().build())
                Logger.info("Handshake successful!")
            } catch (e: Exception) {
                Logger.warn("HandShake failed with the following error: ${e.message}")
            }
        }
    }

    private fun startPeriodicHeartBeat() {
        Logger.info("The heartbeat started with interval: $HEARTBEAT_INTERVAL ms")
        longLivingRequestsCS.launch(CoroutineName("periodicHeartBeat")) {
            while (isActive) {
                heartBeatOnce()
                delay(HEARTBEAT_INTERVAL)
            }
            Logger.info("Stopped heartBeating the server!")
        }
    }

    fun execute(request: Request) {
        shortLivingRequestsCS.launch {
            try {
                request.execute(grpcStub, coroutineContext[Job])
            } catch (e: io.grpc.StatusException) {
                handleGRPCStatusException(e, "Exception when executing server request")
            }
        }
    }

    suspend fun handShake(): Testgen.DummyResponse {
        Logger.info("Sending HANDSHAKE request")
        return grpcStub.handshake(Testgen.DummyRequest.newBuilder().build())
    }

    fun configureProject() {
        CheckProjectConfigurationRequest(
            project,
            getProjectConfigRequestMessage(project, Testgen.ConfigMode.CHECK)
        ).also {
            execute(it)
        }
    }

    private suspend fun heartBeatOnce() {
        try {
            val response = grpcStub.heartbeat(Testgen.DummyRequest.newBuilder().build())
            if (!messageBus.isDisposed) {
                val oldStatus = connectionStatus
                connectionStatus = ConnectionStatus.CONNECTED
                val connectionChangedPublisher = messageBus.syncPublisher(UTBotEventsListener.CONNECTION_CHANGED_TOPIC)
                connectionChangedPublisher.onHeartbeatSuccess(response)
                connectionChangedPublisher.onConnectionChange(oldStatus, ConnectionStatus.CONNECTED)
            }
        } catch (e: io.grpc.StatusException) {
            val oldStatus = connectionStatus
            connectionStatus = ConnectionStatus.BROKEN
            if (!messageBus.isDisposed) {
                val connectionChangedPublisher = messageBus.syncPublisher(UTBotEventsListener.CONNECTION_CHANGED_TOPIC)
                connectionChangedPublisher.onConnectionChange(oldStatus, ConnectionStatus.BROKEN)
            }

            handleGRPCStatusException(e, "Heartbeat failed with grpc io exception")
        }
    }

    private fun handleGRPCStatusException(e: io.grpc.StatusException, message: String) {
        Logger.error(message)
        Logger.error(e.message)
        when (e.status) {
            Status.UNAVAILABLE -> Logger.error("Server is unavailable: possibly it is shut down.")
            Status.UNKNOWN -> Logger.error("Server threw an exception.")
        }
    }

    override fun dispose() {
        Logger.trace("Disposing client!")
        // when project is closed, cancel all running coroutines
        cancelAllRequestsAndWaitForCancellation()
        // release resources associated with grpc
        grpcClient.close()
        Logger.trace("Finished disposing client!")
    }

    fun waitForServerRequestsToFinish(timeout: Long = SERVER_TIMEOUT) {
        runBlocking {
            withTimeout(timeout) {
                while (shortLivingRequestsCS.hasChildren()) {
                    delay(DELAY_TIME)
                }
            }
        }
    }

    private fun cancelAllRequestsAndWaitForCancellation(timeoutMillis: Long = SERVER_TIMEOUT) {
        runBlocking {
            withTimeout(timeoutMillis) {
                while (shortLivingRequestsCS.hasChildren() || longLivingRequestsCS.hasChildren()) {
                    shortLivingRequestsCS.cancel()
                    longLivingRequestsCS.cancel()
                    Logger.trace { "There are unfinished requests:\n${shortLivingRequestsCS.children}\n${longLivingRequestsCS.children}" }
                    Logger.trace("Waiting $DELAY_TIME ms for them to cancel!")
                    delay(DELAY_TIME)
                }
            }
        }
    }

    companion object {
        var IS_TEST_MODE = false
        const val RANDOM_SEQUENCE_MAX_VALUE = 10
        const val RANDOM_SEQUENCE_LENGTH = 5
        const val HEARTBEAT_INTERVAL: Long = 500L
        const val SERVER_TIMEOUT: Long = 300000L
        const val DELAY_TIME: Long = 500L
    }
}
