package server

import org.slf4j.Logger
import org.slf4j.LoggerFactory
import testsgen.Testgen

/**
 * Intercepts incoming messages and logs them.
 *
 * Can be used to see what messages are sent to server for debugging.
 * A listener is attached in [interceptCall], whose [onMessage] method is called
 * when new message is received.
 */
class LogInterceptor : io.grpc.ServerInterceptor {

    companion object {
        val LOG: Logger = LoggerFactory.getLogger(LogInterceptor::class.java)
    }

    override fun <ReqT : Any?, RespT : Any?> interceptCall(
        call: io.grpc.ServerCall<ReqT, RespT>,
        headers: io.grpc.Metadata,
        next: io.grpc.ServerCallHandler<ReqT, RespT>
    ): io.grpc.ServerCall.Listener<ReqT> {
        val logServerCall = LogServerCall(call)
        return object : io.grpc.ForwardingServerCallListener.SimpleForwardingServerCallListener<ReqT>(
            next.startCall(
                logServerCall,
                headers
            )
        ) {
            override fun onMessage(message: ReqT) {
                if (message !is Testgen.DummyRequest)
                    LOG.info("[IN] $message")
                super.onMessage(message)
            }
        }
    }

    private class LogServerCall<ReqT, RestT>(
        delegate: io.grpc.ServerCall<ReqT, RestT>
    ) : io.grpc.ForwardingServerCall.SimpleForwardingServerCall<ReqT, RestT>(delegate) {
        override fun close(status: io.grpc.Status, trailers: io.grpc.Metadata?) {
            if (!status.isOk) {
                LOG.warn("[OUT] code=${status.code}, description=${status.description}, cause=${status.cause.toString()}")
            }
            super.close(status, trailers)
        }
    }
}
