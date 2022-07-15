package com.huawei.utbot.cpp.client.requests

import com.huawei.utbot.cpp.actions.FocusAction
import com.huawei.utbot.cpp.client.Request
import com.huawei.utbot.cpp.client.handlers.TestsStreamHandler
import com.huawei.utbot.cpp.utils.getLongestCommonPathFromRoot
import com.huawei.utbot.cpp.utils.isHeader
import com.huawei.utbot.cpp.utils.logger
import com.huawei.utbot.cpp.utils.notifyInfo
import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt
import java.nio.file.Path

/**
 * Base class for requests.
 * It sends a request of type [X] and handles the response of type [Y].
 */
abstract class BaseRequest<X, Y>(val request: X, val project: Project) : Request {
    abstract val logMessage: String
    override suspend fun execute(stub: TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub, cancellationJob: Job?) {
        logRequest()
        stub.send(cancellationJob).handle(cancellationJob)
    }

    abstract suspend fun TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Y

    abstract suspend fun Y.handle(cancellationJob: Job?)

    open fun logRequest() {
        project.logger.info { "$logMessage \n$request" }
    }
}

/**
 * Base class for requests that handle a stream of [Testgen.TestsResponse].
 * @param progressName - a name of a progress that user will see, when this request will be executing.
 */
abstract class BaseTestsRequest<R>(request: R, project: Project, private val progressName: String) :
    BaseRequest<R, Flow<Testgen.TestsResponse>>(request, project) {
    val logger = project.logger

    override suspend fun Flow<Testgen.TestsResponse>.handle(cancellationJob: Job?) {
        if (cancellationJob?.isActive == true) {
            TestsStreamHandler(
                project,
                this,
                progressName,
                cancellationJob,
                ::notifySuccess,
                ::notifyError
            ).handle()
        }
    }

    open fun getFocusTarget(generatedTestFiles: List<Path>): Path? {
        return generatedTestFiles.filter { !isHeader(it.fileName.toString()) }.getLongestCommonPathFromRoot()
    }

    override fun logRequest() {
        logger.info { "$logMessage \n$request" }
    }

    open fun getInfoMessage() = "Tests generated!"

    open fun notifySuccess(generatedTestFiles: List<Path>) {
        notifyInfo(getInfoMessage(), project, getFocusTarget(generatedTestFiles)?.let {
          FocusAction(it)
        })
    }

    open fun notifyError(cause: Throwable) {}
}
