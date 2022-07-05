package com.huawei.utbot.cpp.client.handlers

import com.huawei.utbot.cpp.utils.createFileAndMakeDirs
import com.huawei.utbot.cpp.utils.logger
import com.huawei.utbot.cpp.utils.refreshAndFindIOFile
import com.huawei.utbot.cpp.utils.utbotSettings
import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import testsgen.Testgen
import testsgen.Util
import java.nio.file.Path
import java.nio.file.Paths

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
            val filePath: String = project.utbotSettings.convertFromRemotePathIfNeeded(sourceCode.filePath)

            if (!isStubs)
                myGeneratedTestFilesLocalFS.add(Paths.get(filePath))

            if (sourceCode.code.isNotEmpty()) {
                project.logger.trace { "Creating generated test file: $filePath." }
                createFileAndMakeDirs(
                    filePath,
                    sourceCode.code
                )
            }

            var infoMessage = "Generated " + if (isStubs) "stub" else "test" + " file"
            if (isGeneratedFileTestSourceFile(filePath))
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
    }

    override fun onException(exception: Throwable) {
        super.onException(exception)
        onError(exception)
    }
}
