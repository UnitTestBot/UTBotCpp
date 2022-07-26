package org.utbot.cpp.clion.plugin.actions

import com.intellij.openapi.actionSystem.AnAction
import com.intellij.openapi.actionSystem.AnActionEvent
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.client.requests.GenerateJsonFilesRequest

class AskServerToGenerateJsonForProjectConfiguration : AnAction(UTBot.message("projectConfigure.generate.json")) {

    override fun actionPerformed(e: AnActionEvent) = GenerateJsonFilesRequest(e).execute()

    override fun update(e: AnActionEvent) {
        e.presentation.isEnabledAndVisible = e.project != null
    }
}
