package com.huawei.utbot.cpp.client.handlers

import com.huawei.utbot.cpp.ui.UTBotRequestProgressIndicator
import com.huawei.utbot.cpp.utils.invokeOnEdt
import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import testsgen.Util

/**
 * Base class for handling stream of server responses that provide Util.progress.
 * The progress is shown via UTBotProgressIndicator.
 */
abstract class StreamHandlerWithProgress<T>(
    project: Project,
    grpcStream: Flow<T>,
    progressName: String,
    cancellationJob: Job
): StreamHandler<T>(project, grpcStream) {
    private val indicator = UTBotRequestProgressIndicator(progressName, cancellationJob, project)

    override fun onStart() {
        super.onStart()
        indicator.start()
    }

    override fun onFinish() {
        super.onFinish()
        invokeOnEdt {
            indicator.complete()
        }
    }

    override fun onData(data: T) {
        updateProgress(data)
    }

    private fun updateProgress(data: T) {
        invokeOnEdt {
            data.getProgress().apply {
                indicator.fraction = percent
                indicator.text = "$message..."
            }
        }
    }

    abstract fun T.getProgress(): Util.Progress

    override fun onException(exception: Throwable) {
        super.onException(exception)
        invokeOnEdt {
            indicator.cancel()
        }
    }
}
