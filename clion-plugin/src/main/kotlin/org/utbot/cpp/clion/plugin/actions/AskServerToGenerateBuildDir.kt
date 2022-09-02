package org.utbot.cpp.clion.plugin.actions

import com.intellij.openapi.actionSystem.AnAction
import com.intellij.openapi.actionSystem.AnActionEvent
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.client.requests.CreateBuildDirRequest

class AskServerToGenerateBuildDir : AnAction(UTBot.message("projectConfigure.generate.buildDir")) {

    override fun actionPerformed(e: AnActionEvent) = CreateBuildDirRequest(e).executeUsingCurrentClient()

    override fun update(e: AnActionEvent) {
        e.presentation.isEnabledAndVisible = e.project != null
    }
}
