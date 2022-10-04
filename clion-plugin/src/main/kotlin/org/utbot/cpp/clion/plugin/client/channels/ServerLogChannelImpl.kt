package org.utbot.cpp.clion.plugin.client.channels

import com.intellij.openapi.components.service
import com.intellij.openapi.project.Project
import kotlinx.coroutines.flow.Flow
import org.utbot.cpp.clion.plugin.grpc.getDummyGrpcRequest
import org.utbot.cpp.clion.plugin.grpc.getLogChannelGrpcRequest
import org.utbot.cpp.clion.plugin.ui.services.OutputProvider
import org.utbot.cpp.clion.plugin.ui.utbotToolWindow.logsToolWindow.UTBotConsole
import testsgen.Testgen
import testsgen.TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub

class ServerLogChannelImpl(project: Project): LogChannelImpl(project) {
    override val name: String = "Server Log"
    override val logLevel: String = "ServerLogLevel"

    override suspend fun open(stub: TestsGenServiceCoroutineStub): Flow<Testgen.LogEntry> =
        stub.openLogChannel(getLogChannelGrpcRequest(logLevel))

    override fun createConsole(): UTBotConsole = project.service<OutputProvider>().serverOutputChannel.outputConsole

    override suspend fun close(stub: TestsGenServiceCoroutineStub) {
        stub.closeLogChannel(getDummyGrpcRequest())
    }
}
