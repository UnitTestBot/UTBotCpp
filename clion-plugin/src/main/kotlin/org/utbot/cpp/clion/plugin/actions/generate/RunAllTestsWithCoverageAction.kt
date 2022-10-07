package org.utbot.cpp.clion.plugin.actions.generate

import com.intellij.openapi.actionSystem.AnActionEvent
import org.utbot.cpp.clion.plugin.client.requests.RunAllTestsWithCoverageRequest
import org.utbot.cpp.clion.plugin.grpc.GrpcRequestBuilderFactory
import org.utbot.cpp.clion.plugin.utils.activeProject

class RunAllTestsWithCoverageAction : BaseGenerateTestsAction() {
    override fun actionPerformed(e: AnActionEvent) {
        RunAllTestsWithCoverageRequest(
            GrpcRequestBuilderFactory(e.activeProject()).createCovAndResulstsRequestBuilder(null),
            e.activeProject()
        ).execute()
    }

    override fun isDefined(e: AnActionEvent): Boolean {
        return e.project != null
    }
}
