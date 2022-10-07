package org.utbot.cpp.clion.plugin.client.requests

import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import org.utbot.cpp.clion.plugin.client.Request
import org.utbot.cpp.clion.plugin.grpc.GrpcRequestBuilder
import org.utbot.cpp.clion.plugin.grpc.RemoteMapping
import org.utbot.cpp.clion.plugin.utils.client
import org.utbot.cpp.clion.plugin.utils.logger
import testsgen.TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub

/**
 * Base class for requests.
 * It sends a request of type [X] and handles the response of type [Y].
 */
abstract class BaseRequest<X, Y>(val params: GrpcRequestBuilder<X>, val project: Project) : Request {
    abstract val logMessage: String
    val request: X by lazy { build() } // must not be accessed from this class constructor

    override fun toString(): String = logMessage

    open fun build(): X {
        val mapping = RemoteMapping(project)
        return params.build(mapping)
    }

    override suspend fun execute(stub: TestsGenServiceCoroutineStub, cancellationJob: Job?) {
        project.logger.info { "$logMessage \n$request" }
        stub.send(cancellationJob).handle(cancellationJob)
    }

    abstract suspend fun TestsGenServiceCoroutineStub.send(cancellationJob: Job?): Y

    abstract suspend fun Y.handle(cancellationJob: Job?)

    fun execute() = project.client.executeRequest(this)

    open fun logRequest() = project.logger.info { "$logMessage \n$request" }
}

