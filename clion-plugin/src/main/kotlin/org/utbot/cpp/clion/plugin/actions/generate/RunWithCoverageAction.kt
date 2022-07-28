package org.utbot.cpp.clion.plugin.actions.generate

import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.CommonDataKeys
import com.intellij.openapi.diagnostic.Logger
import com.intellij.psi.PsiElement
import org.utbot.cpp.clion.plugin.client.requests.RunWithCoverageRequest
import org.utbot.cpp.clion.plugin.grpc.getCoverageAndResultsGrpcRequest
import org.utbot.cpp.clion.plugin.ui.testsResults.TestNameAndTestSuite
import org.utbot.cpp.clion.plugin.utils.activeProject


class RunWithCoverageAction(val element: PsiElement) : BaseGenerateTestsAction() {
    override fun actionPerformed(e: AnActionEvent) {
        logger.debug("Action RunWithCoverageAction was called")

        val testArgs = TestNameAndTestSuite.getFromPsiElement(element)
        val suiteName = testArgs.suite
        val testedMethodName = testArgs.name
        val filePath = e.getRequiredData(CommonDataKeys.VIRTUAL_FILE).path
        val project = e.activeProject()

        RunWithCoverageRequest(
            getCoverageAndResultsGrpcRequest(project, filePath, suiteName, testedMethodName),
            project,
        ).executeUsingCurrentClient()
    }

    override fun isDefined(e: AnActionEvent): Boolean {
        val file = e.getData(CommonDataKeys.PSI_FILE)
        return file != null
    }

    companion object {
        private val logger = Logger.getInstance(this::class.java)
    }
}
