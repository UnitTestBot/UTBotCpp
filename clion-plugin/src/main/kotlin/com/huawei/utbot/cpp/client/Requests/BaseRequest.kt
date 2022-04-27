package com.huawei.utbot.cpp.client.Requests

import com.huawei.utbot.cpp.client.Request
import com.huawei.utbot.cpp.client.handlers.TestsStreamHandler
import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import org.tinylog.kotlin.Logger
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt

/**
 * Base class for requests.
 * It sends a request of type [X] and handles the response of type [Y].
 */
abstract class BaseRequest<X, Y>(val request: X) : Request {
    abstract val logMessage: String
    override suspend fun execute(stub: TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub, cancellationJob: Job?) {
        logRequest()
        stub.send(cancellationJob).handle(cancellationJob)
    }

    abstract suspend fun TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Y

    abstract suspend fun Y.handle(cancellationJob: Job?)

    open fun logRequest() {
        Logger.info(logMessage)
        Logger.trace { "$request" }
    }
}

/**
 * Base class for requests that handle a stream of [Testgen.TestsResponse].
 * @param progressName - a name of a progress that user will see, when this request will be executing.
 */
abstract class BaseTestsRequest<R>(request: R, val project: Project, val progressName: String) :
    BaseRequest<R, Flow<Testgen.TestsResponse>>(request) {

    override suspend fun Flow<Testgen.TestsResponse>.handle(cancellationJob: Job?) {
        if (cancellationJob?.isActive == true) {
            TestsStreamHandler(
                project,
                this,
                progressName,
                cancellationJob
            ).handle()
        }
    }
}
