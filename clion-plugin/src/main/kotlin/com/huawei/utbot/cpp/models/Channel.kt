package com.huawei.utbot.cpp.models

import com.huawei.utbot.cpp.actions.utils.getDummyRequest
import com.huawei.utbot.cpp.actions.utils.getLogChannelRequest
import com.huawei.utbot.cpp.ui.userLog.OutputProvider
import com.huawei.utbot.cpp.ui.userLog.UTBotConsole
import com.huawei.utbot.cpp.utils.invokeOnEdt
import com.huawei.utbot.cpp.utils.logger
import com.intellij.openapi.components.service
import com.intellij.openapi.project.Project
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.cancellable
import kotlinx.coroutines.flow.catch
import kotlinx.coroutines.flow.collect
import kotlinx.coroutines.flow.onEach
import org.tinylog.kotlin.Logger
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt

interface LoggingChannel {
    suspend fun provide(stub: TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub)
}

abstract class BaseChannel(val project: Project): LoggingChannel {
    abstract val name: String
    abstract val logLevel: String
    abstract val console: UTBotConsole
    private val logger = project.logger

    abstract suspend fun close(stub: TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub)
    abstract suspend fun open(stub: TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub): Flow<Testgen.LogEntry>

    open fun log(entry: Testgen.LogEntry) {
        invokeOnEdt {
            console.info(entry.message)
        }
    }

    override fun toString(): String = name

    override suspend fun provide(stub: TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub) {
        try {
            close(stub)
        } catch (cause: io.grpc.StatusException) {
            logger.error{ "Exception when closing log channel: $name \n$cause" }
        }

        open(stub)
            .catch { cause ->
                logger.error{ "Exception in log channel: $name \n$cause" }
            }
            .collect {
                log(it)
            }
    }
}

class GTestChannel(project: Project): BaseChannel(project) {
    override val name: String = "GTest Log"
    override val logLevel: String = "TestLogLevel"
    override val console: UTBotConsole = project.service<OutputProvider>().gtestOutputChannel.outputConsole

    override suspend fun close(stub: TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub) {
        stub.closeGTestChannel(getDummyRequest())
    }

    override suspend fun open(stub: TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub): Flow<Testgen.LogEntry> {
        return stub.openGTestChannel(getLogChannelRequest(logLevel))
    }
}

class ServerLogChannel(project: Project): BaseChannel(project) {
    override val name: String = "Server Log"
    override val logLevel: String = "ServerLogLevel"
    override val console: UTBotConsole = project.service<OutputProvider>().serverOutputChannel.outputConsole

    override suspend fun close(stub: TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub) {
        stub.closeLogChannel(getDummyRequest())
    }

    override suspend fun open(stub: TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub): Flow<Testgen.LogEntry> {
        return stub.openLogChannel(getLogChannelRequest(logLevel))
    }
}
