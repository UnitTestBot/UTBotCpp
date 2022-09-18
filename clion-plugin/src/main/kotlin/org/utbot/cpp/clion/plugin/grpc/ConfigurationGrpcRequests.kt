import testsgen.Testgen

fun getDummyGrpcRequest(): Testgen.DummyRequest = Testgen.DummyRequest.newBuilder().build()

fun getLogChannelGrpcRequest(logLevel: String): Testgen.LogChannelRequest =
    Testgen.LogChannelRequest.newBuilder()
        .setLogLevel(logLevel)
        .build()