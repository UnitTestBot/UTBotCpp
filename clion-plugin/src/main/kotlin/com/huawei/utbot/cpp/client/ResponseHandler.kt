package com.huawei.utbot.cpp.client

import com.huawei.utbot.cpp.actions.AskServerToGenerateBuildDir
import com.huawei.utbot.cpp.actions.AskServerToGenerateJsonForProjectConfiguration
import com.huawei.utbot.cpp.utils.createFileAndMakeDirs
import com.huawei.utbot.cpp.utils.refreshAndFindIOFile
import com.huawei.utbot.cpp.coverage.UTBotCoverageEngine
import com.huawei.utbot.cpp.coverage.UTBotCoverageRunner
import com.huawei.utbot.cpp.coverage.UTBotCoverageSuite
import com.huawei.utbot.cpp.messaging.UTBotTestResultsReceivedListener
import com.huawei.utbot.cpp.services.UTBotSettings
import com.huawei.utbot.cpp.ui.UTBotRequestProgressIndicator
import com.huawei.utbot.cpp.utils.notifyError
import com.huawei.utbot.cpp.utils.notifyInfo
import com.huawei.utbot.cpp.utils.notifyUnknownResponse
import com.intellij.coverage.CoverageDataManager
import com.intellij.coverage.CoverageEngine
import com.intellij.coverage.CoverageRunner
import com.intellij.openapi.application.ApplicationManager
import com.intellij.openapi.components.service
import com.intellij.openapi.diagnostic.Logger
import com.intellij.openapi.project.Project
import kotlin.coroutines.coroutineContext
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.catch
import kotlinx.coroutines.flow.collect
import testsgen.Testgen
import testsgen.Util

/**
 * Handles test responses, and also project configuration responses.
 */
class ResponseHandler(val project: Project, val client: Client) {
    private val utbotSettings: UTBotSettings = project.service()
    private val logger = Logger.getInstance(this::class.java)

    private fun handleTestsResponse(response: Testgen.TestsResponse, uiProgress: UTBotRequestProgressIndicator) {
        if (response.hasProgress()) {
            handleProgress(response.progress, uiProgress)
        }
        handleSourceCode(response.testSourcesList)
        if (response.hasStubs()) {
            handleStubsResponse(response.stubs, uiProgress)
        }
    }

    suspend fun handleCoverageAndResultsResponse(grpcStream: Flow<Testgen.CoverageAndResultsResponse>, uiProgressName: String) {
        fun dataHandler(response: Testgen.CoverageAndResultsResponse, uiProgress: UTBotRequestProgressIndicator) {
            if (response.hasProgress()) {
                ApplicationManager.getApplication().invokeLater {
                    handleProgress(response.progress, uiProgress)
                }
            }
        }
        val lastResponse = handleWithUIProgress(grpcStream, uiProgressName, ::dataHandler)
        lastResponse ?: error("Last response is Null")
        if (lastResponse.errorMessage.isNotEmpty()) {
            notifyError(lastResponse.errorMessage, project)
        }

        // when we received results, test statuses should be updated in the gutter
        project.messageBus.syncPublisher(UTBotTestResultsReceivedListener.TOPIC).testResultsReceived(lastResponse.testRunResultsList)

        logger.debug("LAUNCHING PROCESSING OF COVERAGE")

        logger.debug("com.huawei.utbot.cpp.clion.coverage list size: ${lastResponse.coveragesList.size}")
        lastResponse.coveragesList.forEach {
            logger.info("${it.filePath.substringAfterLast('/')}: ${it.coveredRangesList.size} ${it.uncoveredRangesList.size}")
        }

        logger.debug("test results list size: ${lastResponse.testRunResultsList.size}")
        lastResponse.testRunResultsList.forEach {
            logger.info("${it.testFilePath}: name: ${it.testname}, status: ${it.status}")
        }
        val engine = CoverageEngine.EP_NAME.findExtension(UTBotCoverageEngine::class.java) ?: error("UTBotEngine instance is not found!")
        val coverageRunner = CoverageRunner.getInstance(UTBotCoverageRunner::class.java)
        val manager = CoverageDataManager.getInstance(project)
        val suite = UTBotCoverageSuite(engine,
            lastResponse.coveragesList,
            coverageRunner = coverageRunner,
            name = "UTBot com.huawei.utbot.cpp.clion.coverage suite",
            project = project
        )
        manager.coverageGathered(suite)
    }

    private fun handleStubsResponse(response: Testgen.StubsResponse, uiProgress: UTBotRequestProgressIndicator) {
        if (response.hasProgress()) {
            handleProgress(response.progress, uiProgress)
        }
        handleSourceCode(response.stubSourcesList)
    }

    private fun handleSourceCode(sources: List<Util.SourceCode>) {
        sources.forEach { sourceCode ->
            val filePath: String = utbotSettings.convertFromRemotePathIfNeeded(sourceCode.filePath)
            if (sourceCode.code.isNotEmpty()) {
                createFileAndMakeDirs(
                    filePath,
                    sourceCode.code
                )
            }
            refreshAndFindIOFile(filePath)
        }
    }

    private fun handleProgress(
        serverProgress: Util.Progress,
        uiProgress: UTBotRequestProgressIndicator,
    ) {
        // update progress in status bar
        uiProgress.fraction = serverProgress.percent
        uiProgress.text = serverProgress.message + "..."
    }

    suspend fun handleTestsStream(grpcStream: Flow<Testgen.TestsResponse>, progressName: String) {
        handleWithUIProgress(grpcStream, progressName, this::handleTestsResponse)
        refreshAndFindIOFile(utbotSettings.testDirPath)
    }

    private suspend fun handleProjectConfigResponseStream(
        grpcStream: Flow<Testgen.ProjectConfigResponse>,
        progressName: String,
        onProgressCompletion: (Testgen.ProjectConfigResponse) -> Unit
    ) {
        fun handleLastResponse(response: Testgen.ProjectConfigResponse, uiProgress: UTBotRequestProgressIndicator) {
            if (!response.hasProgress() || response.progress.completed) {
                onProgressCompletion(response)
                return
            }
            handleProgress(response.progress, uiProgress)
        }
        handleWithUIProgress(grpcStream, progressName, ::handleLastResponse)
    }

    suspend fun handleCheckConfigurationResponse(
        grpcStream: Flow<Testgen.ProjectConfigResponse>,
        uiProgressName: String
    ) {
        fun handleProjectConfigCheckResponse(response: Testgen.ProjectConfigResponse) {
            when (response.type) {
                Testgen.ProjectConfigStatus.IS_OK -> {
                    notifyInfo("Project is configured!", project)
                }
                Testgen.ProjectConfigStatus.BUILD_DIR_NOT_FOUND -> {
                    notifyError(
                        "Project build dir not found! ${response.message}",
                        project,
                        AskServerToGenerateBuildDir()
                    )
                }
                Testgen.ProjectConfigStatus.LINK_COMMANDS_JSON_NOT_FOUND, Testgen.ProjectConfigStatus.COMPILE_COMMANDS_JSON_NOT_FOUND -> {
                    val missingFileName =
                        if (response.type == Testgen.ProjectConfigStatus.LINK_COMMANDS_JSON_NOT_FOUND) "link_commands.json" else "compile_commands.json"
                    notifyError(
                        "Project is not configured properly: $missingFileName is missing in the build folder.",
                        project, AskServerToGenerateJsonForProjectConfiguration()
                    )
                }
                else -> notifyUnknownResponse(response, project)
            }
        }

        handleProjectConfigResponseStream(grpcStream, uiProgressName, ::handleProjectConfigCheckResponse)
    }

    suspend fun handleCreateBuildDirResponse(
        grpcStream: Flow<Testgen.ProjectConfigResponse>,
        uiProgressName: String
    ) {
        fun handleBuildDirCreation(serverResponse: Testgen.ProjectConfigResponse) {
            when (serverResponse.type) {
                Testgen.ProjectConfigStatus.IS_OK -> {
                    notifyInfo("Build dir was created!", project)
                    client.configureProject()
                }
                Testgen.ProjectConfigStatus.BUILD_DIR_CREATION_FAILED -> {
                    notifyInfo("Failed to create build dir! ${serverResponse.message}", project)
                }
                else -> notifyUnknownResponse(serverResponse, project)
            }
        }

        handleProjectConfigResponseStream(grpcStream, uiProgressName, ::handleBuildDirCreation)
        refreshAndFindIOFile(utbotSettings.buildDirPath)
    }


    suspend fun handleGenerateJsonResponse(
        grpcStream: Flow<Testgen.ProjectConfigResponse>,
        uiProgressName: String
    ) {
        fun handleJSONGeneration(serverResponse: Testgen.ProjectConfigResponse) {
            when (serverResponse.type) {
                Testgen.ProjectConfigStatus.IS_OK -> notifyInfo("Successfully configured project!", project)
                Testgen.ProjectConfigStatus.RUN_JSON_GENERATION_FAILED -> notifyError(
                    "UTBot tried to configure project, but failed with the " +
                            "following message: ${serverResponse.message}", project
                )
                else -> notifyUnknownResponse(serverResponse, project)
            }
        }
        handleProjectConfigResponseStream(grpcStream, uiProgressName, ::handleJSONGeneration)
        refreshAndFindIOFile(utbotSettings.buildDirPath)
    }


    /**
     * Handle server stream of data messages showing progress in the status bar
     *
     * @param dataHandler - handles the data, and updates progress in the status bar
     * @param uiProgressName - name that will be displayed in the UI for this progress
     * @param grpcStream - stream of data messages
     * @return last received message, if no messages were received - return null
     */
    private suspend fun <T> handleWithUIProgress(
        grpcStream: Flow<T>,
        uiProgressName: String,
        dataHandler: (T, UTBotRequestProgressIndicator) -> Unit,
    ): T? {
        val uiProgress = UTBotRequestProgressIndicator(uiProgressName)
        ApplicationManager.getApplication().invokeLater {
            uiProgress.start()
        }
        uiProgress.requestJob = coroutineContext[Job]
        var lastReceivedData: T? = null
        grpcStream
            .catch { exception ->
                logger.info("In catch of handleWithProgress")
                logger.warn(exception.message)
                exception.message?.let { notifyError(it, project) }
            }
            .collect {
                lastReceivedData = it
                dataHandler(it, uiProgress)
            }
        ApplicationManager.getApplication().invokeLater {
            uiProgress.complete()
        }
        return lastReceivedData
    }
}
