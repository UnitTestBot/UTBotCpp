package org.utbot.cpp.clion.plugin.ui.testsResults

import com.intellij.codeInsight.daemon.LineMarkerInfo
import com.intellij.codeInsight.daemon.LineMarkerProvider
import com.intellij.openapi.actionSystem.AnAction
import com.intellij.openapi.components.service
import com.intellij.openapi.diagnostic.Logger
import com.intellij.openapi.editor.markup.GutterIconRenderer
import com.intellij.psi.PsiElement
import javax.swing.Icon
import org.utbot.cpp.clion.plugin.actions.generate.RunWithCoverageAction
import org.utbot.cpp.clion.plugin.ui.services.TestsResultsStorage

class UTBotTestRunLineMarkerProvider : LineMarkerProvider {
    val log = Logger.getInstance(this::class.java)

    override fun getLineMarkerInfo(element: PsiElement): LineMarkerInfo<*>? {
        if (element.firstChild != null
            //TODO: introduce some alternative to the hardcode of this constants everywhere
            || (element.text != "TEST" && element.text != "UTBot")
            || element.containingFile.name.endsWith(".h")) {
            return null
        }
        val message = if (element.text == "TEST") "UTBot: Run with coverage" else "Run all tests with coverage"
        val icon = element.project.service<TestsResultsStorage>().getTestStatusIcon(element)

        return UTBotRunWithCoverageLineMarkerInfo(element, message, icon)
    }

    private class UTBotRunWithCoverageLineMarkerInfo(
        callElement: PsiElement,
        message: String,
        icon: Icon,
    ): LineMarkerInfo<PsiElement>(
        callElement,
        callElement.textRange,
        icon,
        { message },
        null,
        GutterIconRenderer.Alignment.LEFT,
        { message }
    ) {
        override fun createGutterRenderer(): GutterIconRenderer {
            return object : LineMarkerGutterIconRenderer<PsiElement>(this) {
                override fun isNavigateAction(): Boolean = true
                override fun getClickAction(): AnAction? = element?.let { RunWithCoverageAction(it) }
            }
        }
    }
}