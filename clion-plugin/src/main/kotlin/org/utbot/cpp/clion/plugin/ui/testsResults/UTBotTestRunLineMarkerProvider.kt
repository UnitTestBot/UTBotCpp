package org.utbot.cpp.clion.plugin.ui.testsResults

import com.intellij.codeInsight.daemon.LineMarkerInfo
import com.intellij.codeInsight.daemon.LineMarkerProvider
import com.intellij.icons.AllIcons
import com.intellij.openapi.actionSystem.AnAction
import com.intellij.openapi.components.service
import com.intellij.openapi.diagnostic.Logger
import com.intellij.openapi.editor.markup.GutterIconRenderer
import com.intellij.psi.PsiElement
import javax.swing.Icon
import org.utbot.cpp.clion.plugin.actions.generate.RunWithCoverageAction
import org.utbot.cpp.clion.plugin.ui.services.TestsResultsStorage
import testsgen.Testgen

class UTBotTestRunLineMarkerProvider : LineMarkerProvider {
    val log = Logger.getInstance(this::class.java)

    override fun getLineMarkerInfo(element: PsiElement): LineMarkerInfo<*>? {
        return UTBotRunWithCoverageLineMarkerInfo.createFromPsiElementOrNull(element)
    }

    private class UTBotRunWithCoverageLineMarkerInfo private constructor(
            callElement: PsiElement,
            message: String,
            icon: Icon,
    ) : LineMarkerInfo<PsiElement>(callElement, callElement.textRange, icon, { message }, null, GutterIconRenderer.Alignment.LEFT, { message }) {
        override fun createGutterRenderer(): GutterIconRenderer {
            return object : LineMarkerGutterIconRenderer<PsiElement>(this) {
                override fun isNavigateAction(): Boolean = true
                override fun getClickAction(): AnAction? = element?.let { RunWithCoverageAction(it) }
            }
        }


        companion object {
            fun createFromPsiElementOrNull(element: PsiElement): UTBotRunWithCoverageLineMarkerInfo? {
                if (element.firstChild != null || (!isSingleTest(element) && !isAllTests(element)) || element.containingFile.name.endsWith(".h")) {
                    return null
                }
                val message = if (isSingleTest(element)) "UTBot: Run with coverage" else "Run all tests with coverage"

                return UTBotRunWithCoverageLineMarkerInfo(element, message, getStatusIcon(element))
            }

            private fun getStatusIcon(element: PsiElement): Icon {
                // return icon for Running All Tests
                if (!isAllTests(element)) {
                    return AllIcons.RunConfigurations.TestState.Run_run
                }

                val testName: String = TestNameAndTestSuite.create(element).name
                val testResult: Testgen.TestResultObject? = element.project.service<TestsResultsStorage>().getTestResultByTestName(testName)

                // if there's no info about TestResult with the specified name, return icon for running single test
                if (testResult == null || testName.isEmpty()) {
                    return AllIcons.RunConfigurations.TestState.Run
                }

                // return icon corresponding to cached test status
                return when (testResult.status) {
                    Testgen.TestStatus.TEST_FAILED -> AllIcons.RunConfigurations.TestState.Red2
                    Testgen.TestStatus.TEST_PASSED -> AllIcons.RunConfigurations.TestState.Green2
                    else -> AllIcons.RunConfigurations.TestError
                }
            }
        }
    }

    companion object {
        private const val RUN_SINGLE_TEST_TEXT_MARK = "TEST"
        private const val RUN_ALL_TESTS_TEXT_MARK = "UTBot"
        private fun isSingleTest(element: PsiElement) = element.text == RUN_SINGLE_TEST_TEXT_MARK
        private fun isAllTests(element: PsiElement) = element.text == RUN_ALL_TESTS_TEXT_MARK
    }
}
