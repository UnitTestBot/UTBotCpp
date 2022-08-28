package org.utbot.cpp.clion.plugin.client.handlers.testsStreamHandler

import com.intellij.openapi.components.service
import com.intellij.openapi.progress.ProgressIndicator
import com.intellij.openapi.progress.ProgressManager
import com.intellij.openapi.progress.Task
import com.intellij.openapi.project.Project
import com.intellij.util.io.exists
import com.intellij.util.io.readText
import kotlin.io.path.appendText
import kotlinx.coroutines.CancellationException
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.client.handlers.SourceCode
import org.utbot.cpp.clion.plugin.client.handlers.StreamHandlerWithProgress
import org.utbot.cpp.clion.plugin.settings.settings
import org.utbot.cpp.clion.plugin.ui.services.TestsResultsStorage
import org.utbot.cpp.clion.plugin.utils.createFileWithText
import org.utbot.cpp.clion.plugin.utils.invokeOnEdt
import org.utbot.cpp.clion.plugin.utils.isCMakeListsFile
import org.utbot.cpp.clion.plugin.utils.isSarifReport
import org.utbot.cpp.clion.plugin.utils.logger
import org.utbot.cpp.clion.plugin.utils.markDirtyAndRefresh
import org.utbot.cpp.clion.plugin.utils.nioPath
import org.utbot.cpp.clion.plugin.utils.notifyError
import testsgen.Testgen
import testsgen.Util
import java.io.IOException
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
) : StreamHandlerWithProgress<Testgen.TestsResponse>(project, grpcStream, progressName, cancellationJob) {

    private val myGeneratedTestFilesLocalFS: MutableList<Path> = mutableListOf()
    private var isCMakePresent = false
    private var isSarifPresent = false

    override fun onData(data: Testgen.TestsResponse) {
        super.onData(data)

        // currently testSourcesList contains not only test sourse codes but
        // also some extra files like sarif report, cmake generated file
        // this was done for compatibility
        val sourceCodes = data.testSourcesList.mapNotNull { it.toSourceCodeOrNull() }
        val testSourceCodes = sourceCodes
            .filter { !it.localPath.isSarifReport() && !it.localPath.isCMakeListsFile() }
        handleTestSources(testSourceCodes)

        // for stubs we know that stubSourcesList contains only stub sources
        val stubSourceCodes = data.stubs.stubSourcesList.mapNotNull { it.toSourceCodeOrNull() }
        handleStubSources(stubSourceCodes)

        val sarifReport = sourceCodes.find { it.localPath.isSarifReport() }
        if (sarifReport != null)
            handleSarifReport(sarifReport)

        val cmakeFile = sourceCodes.find { it.localPath.endsWith("CMakeLists.txt") }
        if (cmakeFile != null)
            handleCMakeFile(cmakeFile)

        // for new generated tests remove previous testResults
        project.service<TestsResultsStorage>().clearTestResults(testSourceCodes)
    }

    override fun onFinish() {
        super.onFinish()
        if (!isCMakePresent)
            project.logger.warn("CMake file is missing in the tests response")
        if (!isSarifPresent)
            project.logger.warn("Sarif report is missing in the tests response")
        // tell ide to refresh vfs and refresh project tree
        markDirtyAndRefresh(project.nioPath)
    }

    private fun handleCMakeFile(cmakeSourceCode: SourceCode) {
        isCMakePresent = true
        createFileWithText(cmakeSourceCode.localPath, cmakeSourceCode.content)
        val rootCMakeFile = project.nioPath.resolve("CMakeLists.txt")
        if (!rootCMakeFile.exists()) {
            project.logger.warn("Root CMakeLists.txt file does not exist. Skipping CMake patches.")
            return
        }

        val currentCMakeFileContent = rootCMakeFile.readText()
        val cMakePrinter = CMakePrinter(currentCMakeFileContent)
        invokeOnEdt { // we can show dialog only from edt

            if (!project.settings.storedSettings.isGTestInstalled) {
                val shouldInstallGTestDialog = ShouldInstallGTestDialog(project)

                if (shouldInstallGTestDialog.showAndGet()) {
                    cMakePrinter.addDownloadGTestSection()
                }

                // whether user confirmed that gtest is installed or we added the gtest section, from now on
                // we will assume that gtest is installed
                project.settings.storedSettings.isGTestInstalled = true
            }

            cMakePrinter.addSubdirectory(project.settings.storedSettings.testDirRelativePath)

            // currently we are on EDT, but writing to file better to be done on background thread
            ProgressManager.getInstance().run(object : Task.Backgroundable(project, "Modifying CMakeLists.txt file", false) {
                override fun run(progressIndicator: ProgressIndicator) {
                    try {
                        if (!cMakePrinter.isEmpty)
                            project.nioPath.resolve("CMakeLists.txt").appendText(cMakePrinter.get())
                    } catch (e: IOException) {
                        notifyError(
                            UTBot.message("notify.title.error"),
                            UTBot.message("notify.error.write.to.file", e.message ?: "unknown reason"),
                            project
                        )
                    }
                }
            })
        }
    }

    override fun onCompletion(exception: Throwable?) {
        invokeOnEdt {
            indicator.stopShowingProgressInUI()
        }
        if (exception != null && exception !is CancellationException) {
            throw exception
        } else if (exception !is CancellationException) {
            onSuccess(myGeneratedTestFilesLocalFS)
        }
    }

    private fun handleSarifReport(sarif: SourceCode) {
        isSarifPresent = true
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

    private fun Util.SourceCode.toSourceCodeOrNull(): SourceCode? {
        return try {
            SourceCode(this, project)
        } catch (e: IllegalArgumentException) {
            project.logger.error("Could not convert remote path to local version: bad path: ${this.filePath}")
            null
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
}
