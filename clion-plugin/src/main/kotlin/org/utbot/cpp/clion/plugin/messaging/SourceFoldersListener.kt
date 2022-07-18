package org.utbot.cpp.clion.plugin.messaging

import com.intellij.util.messages.Topic

fun interface SourceFoldersListener {
    companion object {
        val TOPIC = Topic.create(
            "UTBot source folders changed",
            SourceFoldersListener::class.java
        )
    }

    fun sourceFoldersChanged(newSourceFolders: Set<String>)
}
