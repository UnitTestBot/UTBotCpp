package com.huawei.utbot.cpp.actions

import com.huawei.utbot.cpp.actions.utils.getCoverageAndResultsRequest
import com.huawei.utbot.cpp.client.Requests.RunWithCoverageRequest
import com.huawei.utbot.cpp.utils.client
import com.huawei.utbot.cpp.models.TestNameAndTestSuite
import com.huawei.utbot.cpp.utils.execute
import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.diagnostic.Logger
import com.intellij.psi.PsiElement


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
        ).execute(e)
    }

    override fun updateIfServerAvailable(e: AnActionEvent) {}

    companion object {
        private val log = Logger.getInstance(this::class.java)
    }
}