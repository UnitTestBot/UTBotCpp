package org.utbot.cpp.clion.plugin.client.handlers

import com.intellij.openapi.components.service
import com.intellij.openapi.project.Project
import com.intellij.util.io.exists
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import org.utbot.cpp.clion.plugin.settings.settings
import org.utbot.cpp.clion.plugin.ui.services.TestsResultsStorage
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
        val (testSourceCodes, sarifSourceCodes) = data.testSourcesList.map { SourceCode(it, project) }.partition {
            !it.localPath.isSarifReport()
        }
        val stubSourceCodes = data.stubs.stubSourcesList.map { SourceCode(it, project) }

        project.service<TestsResultsStorage>().newTestsGenerated(testSourceCodes)

        sarifSourceCodes.forEach {
            backupPreviousClientSarifReport(it.localPath)
        }

        // if local scenario: server already created files
        if (project.settings.isRemoteScenario) {
            writeSourceCodes(testSourceCodes, "test")
            writeSourceCodes(sarifSourceCodes, "sarif report")
            writeSourceCodes(stubSourceCodes, "stub")
        }

        // prepare list of generated test files for further processing
        myGeneratedTestFilesLocalFS.addAll(testSourceCodes.map { it.localPath })

        // log to user
        testSourceCodes.forEach { sourceCode ->
            val isTestFileSourceFile = sourceCode.localPath.endsWith("_test.cpp")
            if (isTestFileSourceFile)
                project.logger.info {
                    "Generated test with ${sourceCode.regressionMethodsNumber} tests in regression suite" +
                            " and ${sourceCode.errorMethodsNumber} tests in error suite"
                }
            else
                project.logger.info { "Generated test file ${sourceCode.localPath}" }
        }
        sarifSourceCodes.forEach {
            project.logger.info { "Generated SARIF file ${it.localPath}" }
        }

        // tell ide to refresh vfs and refresh project tree
        markDirtyAndRefresh(project.nioPath)
    }

    private fun writeSourceCodes(sourceCodes: List<SourceCode>, fileKind: String) {
        sourceCodes.forEach {
            project.logger.info { "Write $fileKind file ${it.remotePath} to ${it.localPath}" }
            createFileWithText(it.localPath, it.content)
        }
    }

    override fun Testgen.TestsResponse.getProgress(): Util.Progress {
        return progress
    }

    private fun backupPreviousClientSarifReport(oldPath: Path) {
        fun Number.pad2(): String {
            return ("0$this").takeLast(2)
        }

        if (oldPath.exists()) {
            val ctime = Files.getLastModifiedTime(oldPath)
                .toInstant()
                .atZone(ZoneId.systemDefault())
                .toLocalDateTime()
            val newName = "project_code_analysis-" +
                    ctime.year.toString() +
                    (ctime.monthValue + 1).pad2() +
                    ctime.dayOfMonth.pad2() +
                    ctime.hour.pad2() +
                    ctime.minute.pad2() + ctime.second.pad2() + ".sarif";
            val newPath = Paths.get(oldPath.parent.toString(), newName)
            Files.move(oldPath, newPath)
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
