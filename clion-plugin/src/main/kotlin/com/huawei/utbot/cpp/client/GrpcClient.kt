package com.huawei.utbot.cpp.client

import io.grpc.ManagedChannelBuilder
import testsgen.TestsGenServiceGrpcKt

import java.io.Closeable
import java.util.concurrent.TimeUnit

class GrpcClient(port: Int, serverName: String) : Closeable {
    private val channel = ManagedChannelBuilder.forAddress(serverName, port).usePlaintext().build()
    val stub: TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub =
        TestsGenServiceGrpcKt.TestsGenServiceCoroutineStub(channel)

    override fun close() {
        channel.shutdown().awaitTermination(5, TimeUnit.SECONDS)
    }
}
