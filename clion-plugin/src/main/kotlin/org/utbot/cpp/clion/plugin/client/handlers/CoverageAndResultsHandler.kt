package org.utbot.cpp.clion.plugin.client.handlers

import com.intellij.coverage.CoverageDataManager
import com.intellij.coverage.CoverageEngine
import com.intellij.coverage.CoverageRunner
import com.intellij.openapi.project.Project
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.withContext
import org.utbot.cpp.clion.plugin.actions.FocusAction
import org.utbot.cpp.clion.plugin.coverage.UTBotCoverageEngine
import org.utbot.cpp.clion.plugin.coverage.UTBotCoverageRunner
import org.utbot.cpp.clion.plugin.coverage.UTBotCoverageSuite
import org.utbot.cpp.clion.plugin.listeners.UTBotTestResultsReceivedListener
import org.utbot.cpp.clion.plugin.utils.logger
import org.utbot.cpp.clion.plugin.utils.notifyError
import org.utbot.cpp.clion.plugin.utils.notifyInfo
import testsgen.Testgen
import testsgen.Util
import java.nio.file.Path

class CoverageAndResultsHandler(
    project: Project,
    grpcStream: Flow<Testgen.CoverageAndResultsResponse>,
    progressName: String,
    cancellationJob: Job,
    val sourceFilePath: Path? = null
) : StreamHandlerWithProgress<Testgen.CoverageAndResultsResponse>(project, grpcStream, progressName, cancellationJob) {
    override fun Testgen.CoverageAndResultsResponse.getProgress(): Util.Progress = progress

    override suspend fun handle() {
        // Coverage api should be started from background thread
        withContext(Dispatchers.Default) {
            super.handle()
        }
    }

    override fun onLastResponse(response: Testgen.CoverageAndResultsResponse?) {
        if (response == null) {
            project.logger.error { "No responses from server!" }
            return
        }
        if (response.errorMessage.isNotEmpty()) {
            notifyError(response.errorMessage, project)
        }

        // when we received results, test statuses should be updated in the gutter
        project.messageBus.syncPublisher(UTBotTestResultsReceivedListener.TOPIC)
            .testResultsReceived(response.testRunResultsList)

        logCoverageResponse(response)

        val engine = CoverageEngine.EP_NAME.findExtension(UTBotCoverageEngine::class.java)
            ?: error("UTBotEngine instance is not found!")
        val coverageRunner = CoverageRunner.getInstance(UTBotCoverageRunner::class.java)
        val manager = CoverageDataManager.getInstance(project)
        val suite = UTBotCoverageSuite(
            engine,
            response.coveragesList,
            coverageRunner = coverageRunner,
            name = "UTBot coverage suite",
            project = project
        )

        manager.coverageGathered(suite)
        notifyCoverageReceived()
    }

    private fun logCoverageResponse(response: Testgen.CoverageAndResultsResponse) {
        if (response.errorMessage.isNotEmpty())
            project.logger.warn { response.errorMessage }
        if (response.coveragesList.isEmpty())
            project.logger.error { "No coverage received from server!" }
        project.logger.trace {  "coverage list: \n${response.coveragesList}" }
    }

    private fun notifyCoverageReceived() {
        if (sourceFilePath != null) {
            notifyInfo("Coverage received!", project, FocusAction(sourceFilePath))
        }
    }
}
