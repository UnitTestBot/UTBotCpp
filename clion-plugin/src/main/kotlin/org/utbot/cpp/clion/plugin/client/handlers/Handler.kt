package org.utbot.cpp.clion.plugin.client.handlers

/**
 * Handles a particular response/stream of responses returned from server
 */
interface Handler {
    suspend fun handle()
}
