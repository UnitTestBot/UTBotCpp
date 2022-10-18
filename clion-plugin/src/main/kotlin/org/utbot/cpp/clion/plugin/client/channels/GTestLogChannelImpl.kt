package org.utbot.cpp.clion.plugin.client.channels

import com.intellij.openapi.components.service
import com.intellij.openapi.project.Project
import getDummyGrpcRequest
import getLogChannelGrpcRequest
import kotlinx.coroutines.flow.Flow
import org.utbot.cpp.clion.plugin.ui.services.OutputProvider
import org.utbot.cpp.clion.plugin.ui.utbotToolWindow.logsToolWindow.UTBotConsole
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt

class GTestLogChannelImpl(project: Project): LogChannelImpl(project) {
    override val name: String = "GTest Log"
    override val logLevel: String = "TestLogLevel"

    override suspend fun open(stub: TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub): Flow<Testgen.LogEntry> =
        stub.openGTestChannel(getLogChannelGrpcRequest(logLevel))

    override fun createConsole(): UTBotConsole = project.service<OutputProvider>().gTestOutputChannel.outputConsole

    override suspend fun close(stub: TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub) {
        stub.closeGTestChannel(getDummyGrpcRequest())
    }
}