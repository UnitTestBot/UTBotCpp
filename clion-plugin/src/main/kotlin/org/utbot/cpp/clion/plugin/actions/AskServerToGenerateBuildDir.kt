package org.utbot.cpp.clion.plugin.actions

import com.intellij.openapi.actionSystem.AnActionEvent
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.client.requests.CreateBuildDirRequest
import org.utbot.cpp.clion.plugin.grpc.ParamsBuilder
import org.utbot.cpp.clion.plugin.utils.activeProject
import org.utbot.cpp.clion.plugin.utils.client
import testsgen.Testgen

class AskServerToGenerateBuildDir : UTBotBaseAction(UTBot.message("projectConfigure.generate.buildDir")) {

    override fun actionPerformed(e: AnActionEvent) = CreateBuildDirRequest(
        ParamsBuilder(e.activeProject()).buildProjectConfigRequestParams(Testgen.ConfigMode.CREATE_BUILD_DIR),
        e.activeProject(),
        e.activeProject().client
    ).execute()

    override fun updateIfEnabled(e: AnActionEvent) {
        e.presentation.isEnabledAndVisible = e.project != null
    }
}
