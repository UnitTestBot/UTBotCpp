package org.utbot.cpp.clion.plugin.client.handlers

import com.intellij.openapi.project.Project
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.cancellable
import kotlinx.coroutines.flow.collect
import kotlinx.coroutines.flow.onCompletion
import org.utbot.cpp.clion.plugin.utils.notifyError
import java.util.concurrent.CancellationException

/**
 * Base class for handling stream of server responses
 */
abstract class StreamHandler<T>(
    val project: Project,
    val grpcStream: Flow<T>,
) : Handler {
    val logger = com.intellij.openapi.diagnostic.Logger.getInstance(this::class.java)

    override suspend fun handle() {
        var lastResponse: T? = null
        onStart()
        grpcStream.cancellable()
            .onCompletion { cause ->
                onCompletion(cause)
            }
            .collect { data: T ->
                lastResponse = data
                onData(data)
            }
        onLastResponse(lastResponse)
        onFinish()
    }

    open fun onStart() {}

    abstract fun onData(data: T)

    open fun onCompletion(exception: Throwable?) {
        if (exception != null) {
            logger.warn(exception.message)
            if (exception !is CancellationException) {
                exception.printStackTrace()
                exception.message?.let { notifyError(it, project) }
            }
        }
    }

    open fun onLastResponse(response: T?) {}

    open fun onFinish() {}
}
