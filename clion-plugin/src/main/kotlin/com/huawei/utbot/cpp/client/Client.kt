package com.huawei.utbot.cpp.client

import com.huawei.utbot.cpp.actions.utils.getDummyRequest
import com.huawei.utbot.cpp.actions.utils.getProjectConfigRequestMessage
import com.huawei.utbot.cpp.messaging.ConnectionStatus
import com.huawei.utbot.cpp.messaging.UTBotEventsListener
import com.huawei.utbot.cpp.ui.OutputType
import com.huawei.utbot.cpp.ui.UTBotConsole
import com.intellij.openapi.Disposable

import testsgen.Testgen

import com.intellij.openapi.project.Project
import kotlin.random.Random

import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.cancel
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.catch
import kotlinx.coroutines.flow.collect
import kotlinx.coroutines.isActive
import kotlinx.coroutines.launch
import kotlinx.coroutines.swing.Swing
import kotlinx.coroutines.withContext
import testsgen.TestsGenServiceGrpcKt

import ch.qos.logback.classic.Logger
import com.huawei.utbot.cpp.services.UTBotSettings
import com.huawei.utbot.cpp.ui.OutputWindowProvider
import com.intellij.ide.util.RunOnceUtil
import com.intellij.openapi.components.Service
import com.intellij.openapi.components.service
import io.grpc.Status
import mu.KLogger
import mu.KotlinLogging

enum class LogLevel(val id: String) {
    INFO("INFO"), FATAL("FATAL"), ERROR("ERROR"),
    DEBUG("DEBUG"), TRACE("TRACE"), WARN("WARN")
}

@Service
class Client(val project: Project) : Disposable {
    var connectionStatus = ConnectionStatus.INIT
        private set
    private val messageBus = project.messageBus
    private var heartBeatJob: Job? = null
    private val handler = ResponseHandler(project, this)
    private var logLevel: LogLevel = LogLevel.INFO
    private var newClient = true
    private val settings = project.service<UTBotSettings>()
    private val clientID = generateClientID()

    val grpcCoroutineScope: CoroutineScope = CoroutineScope(Dispatchers.Swing)
    private val logger = setupLogger()
    private val grpcStub: TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub = setupGrpcStub()

    init {
        logger.info("Connecting to server on host: ${settings.serverName} , port: ${settings.port}")
        subscribeToEvents()
        startPeriodicHeartBeat()
    }

    private fun setupLogger(): KLogger = KotlinLogging.logger("ClientLogger").also { newLogger ->
        (newLogger.underlyingLogger as Logger).getAppender("ClientAppender").let {
            (it as ClientLogAppender).utBotConsole =
                project.service<OutputWindowProvider>().outputs[OutputType.CLIENT_LOG]
        }
    }

    private fun setupGrpcStub(): TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub {
        val metadata: io.grpc.Metadata = io.grpc.Metadata()
        val stub = GrpcClient(settings.port, settings.serverName).stub
        metadata.put(io.grpc.Metadata.Key.of("clientid", io.grpc.Metadata.ASCII_STRING_MARSHALLER), clientID)
        return io.grpc.stub.MetadataUtils.attachHeaders(stub, metadata)
    }

    private fun subscribeToEvents() {
        project.messageBus.connect(this)
            .subscribe(UTBotEventsListener.CONNECTION_CHANGED_TOPIC, object : UTBotEventsListener {
                override fun onConnectionChange(oldStatus: ConnectionStatus, newStatus: ConnectionStatus) {
                    if (oldStatus != newStatus && newStatus == ConnectionStatus.CONNECTED) {
                        configureProject()
                    }
                }

                override fun onHeartbeatSuccess(response: Testgen.HeartbeatResponse) {
                    RunOnceUtil.runOnceForProject(project, "UTBot: Register client for server") {
                        registerClient(clientID)
                    }

                    if (newClient || !response.linked) {
                        grpcCoroutineScope.launch {
                            provideLogChannel()
                            provideGTestChannel()
                        }
                        newClient = false
                    }
                }
            })
    }

    fun setLoggingLevel(newLevel: LogLevel) {
        logger.info("Setting new log level: ${newLevel.id}")
        logLevel = newLevel
        grpcCoroutineScope.launch {
            provideLogChannel()
        }
    }

    private suspend fun provideGTestChannel() {
        val request = Testgen.LogChannelRequest.newBuilder().setLogLevel("MAX").build()
        try {
            grpcStub.closeGTestChannel(getDummyRequest())
        } catch (e: Exception) {
            logger.error("Exception when closing gtest channel")
            logger.error(e.message)
        }

        val gTestConsole: UTBotConsole = project.service<OutputWindowProvider>().outputs[OutputType.GTEST]!!
        grpcStub.openGTestChannel(request)
            .catch { exception ->
                logger.error("Exception when opening gtest channel")
                logger.error(exception.message)
            }
            .collect {
                gTestConsole.info(it.message)
            }
    }

    private suspend fun provideLogChannel() {
        val request = Testgen.LogChannelRequest.newBuilder().setLogLevel(logLevel.id).build()
        try {
            grpcStub.closeLogChannel(getDummyRequest())
        } catch (e: Exception) {
            logger.error("Exception when closing log channel")
            logger.error(e.message)
        }

        val serverConsole: UTBotConsole = project.service<OutputWindowProvider>().outputs[OutputType.SERVER_LOG]!!
        grpcStub.openLogChannel(request)
            .catch { exception ->
                logger.error("Exception when opening log channel")
                logger.error(exception.message)
            }
            .collect {
                serverConsole.info(it.message)
            }
    }

    private fun generateClientID(): String {
        fun createRandomSequence() = (1..RANDOM_SEQUENCE_LENGTH)
            .joinToString("") { Random.nextInt(0, RANDOM_SEQUENCE_MAX_VALUE).toString() }

        return project.name + (System.getenv("USER") ?: "unknownUser") + createRandomSequence()
    }

    private fun registerClient(clientID: String) {
        grpcCoroutineScope.launch {
            try {
                logger.info("sending REGISTER CLIENT request, clientID == $clientID")
                grpcStub.registerClient(Testgen.RegisterClientRequest.newBuilder().setClientId(clientID).build())
            } catch (e: Exception) {
                logger.error("Register com.huawei.utbot.cpp.clion.client failed: ${e.message}")
            }
        }
    }

    fun isServerAvailable() = connectionStatus == ConnectionStatus.CONNECTED

    fun doHandShake() {
        logger.info("in doHandShake")
        grpcCoroutineScope.launch {
            try {
                grpcStub.handshake(Testgen.DummyRequest.newBuilder().build())
            } catch (e: Exception) {
                logger.warn("HandShake failed with the following error: ${e.message}")
            }
        }
    }

    private fun startPeriodicHeartBeat() {
        logger.info("The heartbeat started with interval: $HEARTBEAT_INTERVAL ms")
        if (heartBeatJob != null) {
            heartBeatJob?.cancel()
        }
        heartBeatJob = grpcCoroutineScope.launch {
            while (isActive) {
                heartBeatOnce()
                delay(HEARTBEAT_INTERVAL)
            }
            logger.info("Stopped heartBeating the server!")
        }
    }

    fun generateForFile(
        request: Testgen.FileRequest
    ) {
        grpcCoroutineScope.launch {
            logger.info("Sending request to generate for FILE: \n$request")
            handler.handleTestsStream(grpcStub.generateFileTests(request), "Generate For File")
        }
    }

    fun generateForLine(
        request: Testgen.LineRequest
    ) {
        grpcCoroutineScope.launch {
            logger.info("Sending request to generate for LINE: \n$request")
            handler.handleTestsStream(grpcStub.generateLineTests(request), "Generate For Line")
        }
    }

    fun generateForPredicate(
        request: Testgen.PredicateRequest
    ) {
        grpcCoroutineScope.launch {
            logger.info("Sending request to generate for PREDICATE: \n$request")
            handler.handleTestsStream(grpcStub.generatePredicateTests(request), "Generate For Predicate")
        }
    }

    fun generateForFunction(
        request: Testgen.FunctionRequest
    ) {
        grpcCoroutineScope.launch {
            logger.info("Sending request to generate for FUNCTION: \n$request")
            handler.handleTestsStream(grpcStub.generateFunctionTests(request), "Generate For Function")
        }
    }

    fun generateForClass(
        request: Testgen.ClassRequest
    ) {
        grpcCoroutineScope.launch {
            logger.info("Sending request to generate for CLASS: \n$request")
            handler.handleTestsStream(grpcStub.generateClassTests(request), "Generate For Folder")
        }
    }

    fun generateForFolder(
        request: Testgen.FolderRequest
    ) {
        grpcCoroutineScope.launch {
            logger.info("Sending request to generate for FOLDER: \n$request")
            handler.handleTestsStream(grpcStub.generateFolderTests(request), "Generate For Folder")
        }
    }

    fun generateForProject(
        request: Testgen.ProjectRequest
    ) {
        grpcCoroutineScope.launch {
            logger.info("Sending request to generate for PROJECT: \n$request")
            handler.handleTestsStream(grpcStub.generateProjectTests(request), "Generate for Project")
        }
    }

    fun requestProjectTargetsAndProcess(
        request: Testgen.ProjectTargetsRequest,
        callback: (Testgen.ProjectTargetsResponse) -> Unit
    ) {
        grpcCoroutineScope.launch {
            logger.info("Sending request to get PROJECT TARGETS: \n$request")
            val targets = grpcStub.getProjectTargets(request)
            callback(targets)
        }
    }

    fun generateForSnippet(
        request: Testgen.SnippetRequest
    ) {
        grpcCoroutineScope.launch {
            logger.info("Sending request to generate for SNIPPET: \n$request")
            handler.handleTestsStream(grpcStub.generateSnippetTests(request), "Generate For Snippet")
        }
    }

    fun generateForAssertion(
        request: Testgen.AssertionRequest
    ) {
        grpcCoroutineScope.launch {
            logger.info("Sending request to generate for ASSERTION: \n$request")
            handler.handleTestsStream(grpcStub.generateAssertionFailTests(request), "Generate For Assertion")
        }
    }

    fun requestFunctionReturnTypeAndProcess(request: Testgen.FunctionRequest, callback: (Testgen.FunctionTypeResponse) -> Unit) {
        grpcCoroutineScope.launch {
            callback(getFunctionReturnType(request))
        }
    }

    private suspend fun getFunctionReturnType(
        request: Testgen.FunctionRequest
    ): Testgen.FunctionTypeResponse = withContext(Dispatchers.IO) {
        logger.info("Sending request to get FUNCTION RETURN TYPE: \n$request")
        grpcStub.getFunctionReturnType(request)
    }

    suspend fun handShake(): Testgen.DummyResponse {
        logger.info("Sending HANDSHAKE request")
        return grpcStub.handshake(Testgen.DummyRequest.newBuilder().build())
    }

    fun configureProject() {
        val request = getProjectConfigRequestMessage(project, Testgen.ConfigMode.CHECK)
        grpcCoroutineScope.launch {
            logger.info("Sending request to CHECK PROJECT CONFIGURATION: \n$request")
            handler.handleCheckConfigurationResponse(
                grpcStub.configureProject(request),
                "Checking project configuration..."
            )
        }
    }

    fun createBuildDir() {
        val request = getProjectConfigRequestMessage(project, Testgen.ConfigMode.CREATE_BUILD_DIR)
        grpcCoroutineScope.launch {
            logger.info("Sending request to GENERATE BUILD DIR: \n$request")
            handler.handleCreateBuildDirResponse(grpcStub.configureProject(request), "Create build directory...")
        }
    }

    fun getCoverageAndResults(request: Testgen.CoverageAndResultsRequest) {
        grpcCoroutineScope.launch {
            withContext(Dispatchers.Default) {
                logger.info("Sending request to get COVERAGE AND RESULTS: \n$request")
                handler.handleCoverageAndResultsResponse(
                    grpcStub.createTestsCoverageAndResult(request),
                    "Run Tests with Coverage"
                )
            }
        }
    }

    fun generateJSon() {
        val request = getProjectConfigRequestMessage(project, Testgen.ConfigMode.GENERATE_JSON_FILES)
        grpcCoroutineScope.launch {
            logger.info("Sending request to GENERATE JSON FILES: \n$request")
            handler.handleGenerateJsonResponse(grpcStub.configureProject(request), "Generate JSON files...")
        }
    }

    private suspend fun heartBeatOnce() {
        try {
            val response = grpcStub.heartbeat(Testgen.DummyRequest.newBuilder().build())
            if (!messageBus.isDisposed) {
                val oldStatus = connectionStatus
                connectionStatus = ConnectionStatus.CONNECTED
                val connectionChangedPublisher = messageBus.syncPublisher(UTBotEventsListener.CONNECTION_CHANGED_TOPIC)
                connectionChangedPublisher.onConnectionChange(oldStatus, ConnectionStatus.CONNECTED)
                connectionChangedPublisher.onHeartbeatSuccess(response)
            }
        } catch (e: Exception) {
            logger.error("Heartbeat failed with exception: \n${e.message}")
            val oldStatus = connectionStatus
            connectionStatus = ConnectionStatus.BROKEN
            if (!messageBus.isDisposed) {
                val connectionChangedPublisher = messageBus.syncPublisher(UTBotEventsListener.CONNECTION_CHANGED_TOPIC)
                connectionChangedPublisher.onConnectionChange(oldStatus, ConnectionStatus.BROKEN)
            }
        }
    }

    private fun handleGRPCStatusException(e: io.grpc.StatusException) {
        logger.error("Exception when closing log and GTest channels")
        logger.error(e.message)
        when (e.status) {
            Status.UNAVAILABLE -> logger.error("Server is unavailable: possibly it is shut down.")
            Status.UNKNOWN -> logger.error("Server threw an exception.")
        }
    }

    override fun dispose() {
        // when project is closed, cancel all running coroutines
        grpcCoroutineScope.launch {
            try {
                grpcStub.closeLogChannel(getDummyRequest())
                grpcStub.closeGTestChannel(getDummyRequest())
            } catch (e: io.grpc.StatusException) {
                handleGRPCStatusException(e)
            }
            cancel()
        }
    }

    companion object {
        const val RANDOM_SEQUENCE_MAX_VALUE = 10
        const val RANDOM_SEQUENCE_LENGTH = 5
        const val HEARTBEAT_INTERVAL: Long = 500L
    }
}
