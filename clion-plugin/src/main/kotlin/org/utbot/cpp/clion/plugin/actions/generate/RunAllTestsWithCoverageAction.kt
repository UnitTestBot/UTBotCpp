package org.utbot.cpp.clion.plugin.actions.generate

import com.intellij.openapi.actionSystem.AnActionEvent
import org.utbot.cpp.clion.plugin.client.requests.RunAllTestsWithCoverageRequest

class RunAllTestsWithCoverageAction: BaseGenerateTestsAction() {
    override fun actionPerformed(e: AnActionEvent) {
        RunAllTestsWithCoverageRequest(e).execute()
    }

    override fun isDefined(e: AnActionEvent): Boolean {
        return e.project != null
    }
}
