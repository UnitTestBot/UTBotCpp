package org.utbot.cpp.clion.plugin.client

import io.grpc.ManagedChannelBuilder
import testsgen.TestsGenServiceGrpcKt

import java.io.Closeable
import java.util.concurrent.TimeUnit

open class GrpcClient(val port: Int, val serverName: String, val clientId: String) : Closeable {
    private val channel = ManagedChannelBuilder.forAddress(serverName, port).usePlaintext().build()
    val stub: TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub = setupGrpcStub()

    private fun setupGrpcStub(): TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub {
        val metadata: io.grpc.Metadata = io.grpc.Metadata()
        metadata.put(io.grpc.Metadata.Key.of("clientId", io.grpc.Metadata.ASCII_STRING_MARSHALLER), clientId)
        return io.grpc.stub.MetadataUtils.attachHeaders(
            TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub(channel),
            metadata
        )
    }

    override fun close() {
        channel.shutdownNow().awaitTermination(5L, TimeUnit.SECONDS)
    }
}
