package org.utbot.cpp.clion.plugin.actions

import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.diagnostic.Logger
import com.intellij.psi.PsiElement
import org.utbot.cpp.clion.plugin.utils.getCoverageAndResultsRequest
import org.utbot.cpp.clion.plugin.client.requests.RunWithCoverageRequest
import org.utbot.cpp.clion.plugin.ui.testsResults.TestNameAndTestSuite


class RunWithCoverageAction(val element: PsiElement) : GenerateTestsBaseAction() {
    override fun actionPerformed(e: AnActionEvent) {
        log.debug("Action RunWithCoverageAction was called")
        if (element.containingFile == null)
            return
        log.debug("psi element is valid: containing file not null")
        val testArgs = TestNameAndTestSuite.getFromPsiElement(element)
        val suiteName = testArgs.suite
        val testedMethodName = testArgs.name
        val request = getCoverageAndResultsRequest(e, suiteName, testedMethodName)
        RunWithCoverageRequest(
            e.project!!,
            request
        ).execute()
    }

    override fun updateIfServerAvailable(e: AnActionEvent) {}

    companion object {
        private val log = Logger.getInstance(this::class.java)
    }
}
