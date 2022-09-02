package org.utbot.cpp.clion.plugin.client

import kotlinx.coroutines.Job
import testsgen.TestsGenServiceGrpcKt

/**
 * Represents a request to server.
 */
interface Request {
    /**
     * Send a request to server using [stub] and handle the response.
     * @param stub - stub to send requests to server.
     * @param cancellationJob - Job of an enclosing coroutine, to cancel the coroutine if this
     * request can't be completed, for example because user canceled it from UI.
     */
    suspend fun execute(stub: TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub, cancellationJob: Job?)
}
