package org.utbot.cpp.clion.plugin.client.handlers

import com.intellij.coverage.CoverageDataManager
import com.intellij.coverage.CoverageEngine
import com.intellij.coverage.CoverageRunner
import com.intellij.openapi.actionSystem.AnAction
import com.intellij.openapi.project.Project
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.withContext
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.actions.FocusAction
import org.utbot.cpp.clion.plugin.coverage.Coverage
import org.utbot.cpp.clion.plugin.coverage.UTBotCoverageEngine
import org.utbot.cpp.clion.plugin.coverage.UTBotCoverageRunner
import org.utbot.cpp.clion.plugin.coverage.UTBotCoverageSuite
import org.utbot.cpp.clion.plugin.listeners.UTBotTestResultsReceivedListener
import org.utbot.cpp.clion.plugin.utils.convertFromRemotePathIfNeeded
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
            notifyError(
                UTBot.message("notify.title.error"),
                response.errorMessage,
                project
            )
        }

        data class CoverageCollector(
            val fullyCovered: MutableSet<Int> = mutableSetOf(),
            val partiallyCovered: MutableSet<Int> = mutableSetOf(),
            val notCovered: MutableSet<Int> = mutableSetOf()
        ) {
            fun toCoverage() = Coverage(fullyCovered, partiallyCovered, notCovered)
        }

        val coverage = mutableMapOf<Path, CoverageCollector>()
        response.coveragesList.forEach { fileCoverageSimplified ->
            val local = fileCoverageSimplified.filePath.convertFromRemotePathIfNeeded(project).normalize()
            if (local !in coverage)
                coverage[local] = CoverageCollector()
            fileCoverageSimplified.fullCoverageLinesList.forEach { sourceLine ->
                coverage[local]?.fullyCovered?.add(sourceLine.line)
            }
            fileCoverageSimplified.partialCoverageLinesList.forEach { sourceLine ->
                coverage[local]?.partiallyCovered?.add(sourceLine.line)
            }
            fileCoverageSimplified.noCoverageLinesList.forEach { sourceLine ->
                coverage[local]?.notCovered?.add(sourceLine.line)
            }
        }

        // when we received results, test statuses should be updated in the gutter
        project.messageBus.let { bus ->
            if (!bus.isDisposed)
                bus.syncPublisher(UTBotTestResultsReceivedListener.TOPIC)
                    .testResultsReceived(response.testRunResultsList)
        }

        val engine = CoverageEngine.EP_NAME.findExtension(UTBotCoverageEngine::class.java)
            ?: error("UTBotEngine instance is not found!")
        val coverageRunner = CoverageRunner.getInstance(UTBotCoverageRunner::class.java)
        val manager = CoverageDataManager.getInstance(project)
        val suite = UTBotCoverageSuite(
            coverage.mapValues { it.value.toCoverage() },
            engine,
            response.coveragesList,
            coverageRunner = coverageRunner,
            name = "UTBot coverage suite",
            project = project,
        )

        manager.coverageGathered(suite)
        notifyCoverageReceived()
    }

    private fun notifyCoverageReceived() {
        val actions = mutableListOf<AnAction>()
        sourceFilePath?.let { actions.add(FocusAction(it)) }
        notifyInfo(
            UTBot.message("notify.coverage.received.title"),
            UTBot.message("notify.coverage.received"),
            project,
            sourceFilePath?.let { FocusAction(it) }
        )
    }
}
