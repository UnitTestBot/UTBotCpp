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
import kotlin.io.path.name
import org.utbot.cpp.clion.plugin.actions.generate.RunWithCoverageAction
import org.utbot.cpp.clion.plugin.grpc.IllegalPathException
import org.utbot.cpp.clion.plugin.settings.settings
import org.utbot.cpp.clion.plugin.ui.services.TestsResultsStorage
import org.utbot.cpp.clion.plugin.utils.localPath
import org.utbot.cpp.clion.plugin.utils.logger
import testsgen.Testgen

class UTBotTestRunLineMarkerProvider : LineMarkerProvider {
    val log = Logger.getInstance(this::class.java)

    override fun getLineMarkerInfo(element: PsiElement): LineMarkerInfo<*>? =
        UTBotRunWithCoverageLineMarkerInfo.createFromPsiElementOrNull(element)

    private class UTBotRunWithCoverageLineMarkerInfo
    private constructor(callElement: PsiElement, message: String, icon: Icon) : LineMarkerInfo<PsiElement>(
        callElement,
        callElement.textRange,
        icon,
        { message },
        null,
        GutterIconRenderer.Alignment.LEFT,
        { message }) {

        override fun createGutterRenderer(): GutterIconRenderer {
            return object : LineMarkerGutterIconRenderer<PsiElement>(this) {
                override fun isNavigateAction(): Boolean = true
                override fun getClickAction(): AnAction? = element?.let { RunWithCoverageAction(it) }
            }
        }

        companion object {
            fun createFromPsiElementOrNull(element: PsiElement): UTBotRunWithCoverageLineMarkerInfo? {
                val elementRequiresIcon = element.canPlaceSingleTestIcon() || element.canPlaceAllTestsIcon()
                if (element.firstChild != null
                    || !elementRequiresIcon
                    || element.containingFile.name.endsWith(".h")
                    || !isElementInTestFileGeneratedByUTBot(element)
                ) {
                    return null
                }
                val message =
                    if (element.canPlaceSingleTestIcon()) "UTBot: Run with coverage" else "Run all tests with coverage"

                return UTBotRunWithCoverageLineMarkerInfo(element, message, getStatusIcon(element))
            }

            private fun isElementInTestFileGeneratedByUTBot(element: PsiElement) =
                element.containingFile.virtualFile.localPath.let {
                    try {
                        it.toString().startsWith(element.project.settings.testsDirPath.toString()) &&
                                it.name.contains("test")
                    } catch (e: IllegalPathException) {
                        return false
                    }
                }


            fun getStatusIcon(element: PsiElement): Icon {
                // return icon for Running All Tests
                if (element.canPlaceAllTestsIcon()) {
                    return AllIcons.RunConfigurations.TestState.Run_run
                }

                //TODO: it is a little strange to create smth just for a testName
                val testName: String = TestNameAndTestSuite.create(element).name
                val testResult: Testgen.TestResultObject? =
                    element.project.service<TestsResultsStorage>().getTestResultByTestName(testName)

                // if there's no info about TestResult with the specified name, return icon for running single test
                if (testResult == null || testName.isEmpty()) {
                    return AllIcons.RunConfigurations.TestState.Run
                }

                // return icon corresponding to cached test status
                return when (testResult.status) {
                    Testgen.TestStatus.TEST_FAILED -> AllIcons.RunConfigurations.TestState.Red2
                    Testgen.TestStatus.TEST_PASSED -> AllIcons.RunConfigurations.TestState.Green2
                    Testgen.TestStatus.TEST_DEATH,
                    Testgen.TestStatus.TEST_INTERRUPTED,
                    Testgen.TestStatus.UNRECOGNIZED -> AllIcons.RunConfigurations.TestError
                }
            }
        }
    }

    companion object {
        private const val RUN_SINGLE_TEST_TEXT_MARK = "TEST"
        private const val RUN_ALL_TESTS_TEXT_MARK = "UTBot"

        private fun PsiElement.canPlaceSingleTestIcon() = this.text == RUN_SINGLE_TEST_TEXT_MARK
        private fun PsiElement.canPlaceAllTestsIcon() = this.text == RUN_ALL_TESTS_TEXT_MARK
    }
}
