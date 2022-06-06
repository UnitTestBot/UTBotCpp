package com.huawei.utbot.cpp.client.handlers

import com.huawei.utbot.cpp.actions.FocusAction
import com.huawei.utbot.cpp.coverage.UTBotCoverageEngine
import com.huawei.utbot.cpp.coverage.UTBotCoverageRunner
import com.huawei.utbot.cpp.coverage.UTBotCoverageSuite
import com.huawei.utbot.cpp.messaging.UTBotTestResultsReceivedListener
import com.huawei.utbot.cpp.utils.logger
import com.huawei.utbot.cpp.utils.notifyError
import com.huawei.utbot.cpp.utils.notifyInfo
import com.huawei.utbot.cpp.utils.utbotSettings
import com.intellij.coverage.CoverageDataManager
import com.intellij.coverage.CoverageEngine
import com.intellij.coverage.CoverageRunner
import com.intellij.openapi.project.Project
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.withContext
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
        notify(response)
    }

    private fun notify(reponse: Testgen.CoverageAndResultsResponse) {
        sourceFilePath ?: return
        notifyInfo(
            "Coverage received!", project,
            FocusAction(sourceFilePath)
        )
    }
}
