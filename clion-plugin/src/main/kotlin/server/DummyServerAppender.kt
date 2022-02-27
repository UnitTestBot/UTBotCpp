package server

import ch.qos.logback.classic.encoder.PatternLayoutEncoder
import ch.qos.logback.classic.spi.ILoggingEvent
import ch.qos.logback.core.AppenderBase
import java.io.IOException

class DummyServerAppender : AppenderBase<ILoggingEvent?>() {
    var encoder: PatternLayoutEncoder? = null
    var messageBuffer: MutableList<String>? = null

    override fun start() {
        if (encoder == null) {
            addError("No encoder set for the appender named [$name].")
            return
        }
        try {
            encoder?.start()
        } catch (e: IOException) {
        }
        super.start()
    }


    override fun append(event: ILoggingEvent?) {
        // output the events as formatted by our layout
        val logMessage = encoder?.encode(event)?.decodeToString()
        logMessage?.let {
            messageBuffer?.add(logMessage)
        }
    }
}