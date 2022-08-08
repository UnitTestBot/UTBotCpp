package org.utbot.cpp.clion.plugin.client.handlers

import com.intellij.openapi.project.Project
import com.intellij.util.io.exists
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import org.utbot.cpp.clion.plugin.utils.convertFromRemotePathIfNeeded
import org.utbot.cpp.clion.plugin.utils.createFileWithText
import org.utbot.cpp.clion.plugin.utils.isSarifReport
import org.utbot.cpp.clion.plugin.utils.logger
import org.utbot.cpp.clion.plugin.utils.refreshAndFindNioFile
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

    override suspend fun onData(data: Testgen.TestsResponse) {
        super.onData(data)
        handleSourceCode(data.testSourcesList)
        if (data.hasStubs()) {
            handleSourceCode(data.stubs.stubSourcesList, true)
        }
    }

    override fun Testgen.TestsResponse.getProgress(): Util.Progress {
        return progress
    }

    private fun handleSourceCode(sources: List<Util.SourceCode>, isStubs: Boolean = false) {
        sources.forEach { sourceCode ->
            val filePath: Path = sourceCode.filePath.convertFromRemotePathIfNeeded(project)

            if (!isStubs && !isSarifReport(filePath))
                myGeneratedTestFilesLocalFS.add(filePath)

            if (sourceCode.code.isNotEmpty()) {
                project.logger.trace { "Creating generated test file: $filePath." }
                createFileWithText(
                    filePath,
                    sourceCode.code
                )
            }

            var infoMessage = "Generated " + if (isStubs) "stub" else "test" + " file"
            if (isGeneratedFileTestSourceFile(filePath.toString()))
                infoMessage += " with ${sourceCode.regressionMethodsNumber} tests in regression suite" +
                        " and ${sourceCode.errorMethodsNumber} tests in error suite"
            project.logger.info { "$infoMessage: $filePath" }

            refreshAndFindNioFile(filePath)
        }
    }

    fun backupPreviousClientSarifReport(localPath: String) {
        fun Number.pad2(): String {
            return ("0$this").takeLast(2)
        }

        val oldPath = Paths.get(localPath)
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

    private fun isGeneratedFileTestSourceFile(fileName: String) = fileName.endsWith("_test.cpp")

    override fun onCompletion(exception: Throwable?) {
        super.onCompletion(exception)
        if (exception == null)
            onSuccess(myGeneratedTestFilesLocalFS)
        else
            onError(exception)
    }
}
