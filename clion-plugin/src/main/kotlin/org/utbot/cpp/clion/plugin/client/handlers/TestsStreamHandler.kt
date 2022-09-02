package org.utbot.cpp.clion.plugin.client.handlers

import com.intellij.openapi.components.service
import com.intellij.openapi.project.Project
import com.intellij.util.io.exists
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import org.utbot.cpp.clion.plugin.settings.settings
import org.utbot.cpp.clion.plugin.ui.services.TestsResultsStorage
import org.utbot.cpp.clion.plugin.utils.convertFromRemotePathIfNeeded
import org.utbot.cpp.clion.plugin.utils.createFileWithText
import org.utbot.cpp.clion.plugin.utils.isSarifReport
import org.utbot.cpp.clion.plugin.utils.logger
import org.utbot.cpp.clion.plugin.utils.markDirtyAndRefresh
import org.utbot.cpp.clion.plugin.utils.nioPath
import testsgen.Testgen
import testsgen.Util
import java.nio.file.Files
import java.nio.file.Path
import java.nio.file.Paths
import java.time.ZoneId

class TestsStreamHandler(
    project: Project,
    grpcStream: Flow<Testgen.TestsResponse>,
    progressName: String,
    cancellationJob: Job,
    private val onSuccess: (List<Path>) -> Unit = {},
    private val onError: (Throwable) -> Unit = {}
) : StreamHandlerWithProgress<Testgen.TestsResponse>(project, grpcStream, progressName, cancellationJob) {

    private val myGeneratedTestFilesLocalFS: MutableList<Path> = mutableListOf()

    override fun onData(data: Testgen.TestsResponse) {
        super.onData(data)

        val testSourceCodes = data.testSourcesList
            .map { SourceCode(it, project) }
            .filter { !it.localPath.isSarifReport() }
        handleTestSources(testSourceCodes)

        val stubSourceCodes = data.stubs.stubSourcesList.map { SourceCode(it, project) }
        handleStubSources(stubSourceCodes)

        val sarifReport =
            data.testSourcesList.find { it.filePath.convertFromRemotePathIfNeeded(project).isSarifReport() }?.let {
                SourceCode(it, project)
            }
        sarifReport?.let { handleSarifReport(it) }

        // for new generated tests remove previous testResults
        project.service<TestsResultsStorage>().clearTestResults(testSourceCodes)
    }

    override fun onFinish() {
        super.onFinish()
        // tell ide to refresh vfs and refresh project tree
        markDirtyAndRefresh(project.nioPath)
    }

    private fun handleSarifReport(sarif: SourceCode) {
        backupPreviousClientSarifReport(sarif.localPath)
        createSourceCodeFiles(listOf(sarif), "sarif report")
        project.logger.info { "Generated SARIF report file ${sarif.localPath}" }
    }

    private fun handleTestSources(sources: List<SourceCode>) {
        if (project.settings.isRemoteScenario) {
            createSourceCodeFiles(sources, "test")
        }

        // prepare list of generated test files for further processing
        myGeneratedTestFilesLocalFS.addAll(sources.map { it.localPath })

        sources.forEach { sourceCode ->
            val isTestSourceFile = sourceCode.localPath.endsWith("_test.cpp")
            val testsGenerationResultMessage = if (isTestSourceFile) {
                "Generated ${sourceCode.regressionMethodsNumber} tests in regression suite" +
                        " and ${sourceCode.errorMethodsNumber} tests in error suite"
            } else {
                // .h file
                "Generated test file ${sourceCode.localPath}"
            }
            logger.info(testsGenerationResultMessage)
        }
    }

    private fun handleStubSources(sources: List<SourceCode>) {
        if (project.settings.isRemoteScenario) {
            createSourceCodeFiles(sources, "stub")
        }
    }

    private fun createSourceCodeFiles(sourceCodes: List<SourceCode>, fileKind: String) {
        sourceCodes.forEach {
            project.logger.info { "Write $fileKind file ${it.remotePath} to ${it.localPath}" }
            createFileWithText(it.localPath, it.content)
        }
    }

    override fun Testgen.TestsResponse.getProgress(): Util.Progress = progress

    private fun backupPreviousClientSarifReport(previousReportPaths: Path) {
        fun Number.pad2(): String = ("0$this").takeLast(2)

        if (previousReportPaths.exists()) {
            val ctime = Files.getLastModifiedTime(previousReportPaths)
                .toInstant()
                .atZone(ZoneId.systemDefault())
                .toLocalDateTime()

            val newReportName = "project_code_analysis-" +
                    ctime.year.toString() +
                    (ctime.monthValue + 1).pad2() +
                    ctime.dayOfMonth.pad2() +
                    ctime.hour.pad2() +
                    ctime.minute.pad2() +
                    ctime.second.pad2() +
                    ".sarif"
            val newPath = Paths.get(previousReportPaths.parent.toString(), newReportName)
            Files.move(previousReportPaths, newPath)
        }
    }

    override fun onCompletion(exception: Throwable?) {
        super.onCompletion(exception)
        if (exception == null)
            onSuccess(myGeneratedTestFilesLocalFS)
        else
            onError(exception)
    }
}
