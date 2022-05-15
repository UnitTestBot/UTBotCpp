package com.huawei.utbot.cpp.client.handlers

import com.huawei.utbot.cpp.utils.notifyError
import com.intellij.openapi.project.Project
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.cancellable
import kotlinx.coroutines.flow.collect
import kotlinx.coroutines.flow.catch
import kotlinx.coroutines.flow.onCompletion

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
            .catch { exception ->
                onException(exception)
            }
            .collect { data: T ->
                lastResponse = data
                onData(data)
            }
        onLastResponse(lastResponse)
        onFinish()
    }

    open fun onStart() {}

    open fun onException(exception: Throwable) {
        logger.warn(exception.message)
        exception.message?.let { notifyError(it, project) }
    }

    abstract fun onData(data: T)

    open fun onCompletion(exception: Throwable?) {}

    open fun onLastResponse(response: T?) {}

    open fun onFinish() {}
}
