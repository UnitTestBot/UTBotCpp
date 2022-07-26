package org.utbot.cpp.clion.plugin.client.handlers

import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import org.utbot.cpp.clion.plugin.utils.convertFromRemotePathIfNeeded
import org.utbot.cpp.clion.plugin.utils.createFileAndMakeDirs
import org.utbot.cpp.clion.plugin.utils.logger
import org.utbot.cpp.clion.plugin.utils.refreshAndFindIOFile
import testsgen.Testgen
import testsgen.Util
import java.nio.file.Path

class TestsStreamHandler(
    project: Project,
    grpcStream: Flow<Testgen.TestsResponse>,
    progressName: String,
    cancellationJob: Job,
    private val onSuccess: (List<Path>)->Unit = {},
    private val onError: (Throwable)->Unit = {}
): StreamHandlerWithProgress<Testgen.TestsResponse>(project, grpcStream, progressName, cancellationJob) {
    private val myGeneratedTestFilesLocalFS: MutableList<Path> = mutableListOf()

    override fun onData(data: Testgen.TestsResponse) {
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

            if (!isStubs)
                myGeneratedTestFilesLocalFS.add(filePath)

            if (sourceCode.code.isNotEmpty()) {
                project.logger.trace { "Creating generated test file: $filePath." }
                createFileAndMakeDirs(
                    filePath,
                    sourceCode.code
                )
            }

            var infoMessage = "Generated " + if (isStubs) "stub" else "test" + " file"
            if (isGeneratedFileTestSourceFile(filePath.toString()))
                infoMessage += " with ${sourceCode.regressionMethodsNumber} tests in regression suite" +
                        " and ${sourceCode.errorMethodsNumber} tests in error suite"
            project.logger.info { "$infoMessage: $filePath" }

            refreshAndFindIOFile(filePath)
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
