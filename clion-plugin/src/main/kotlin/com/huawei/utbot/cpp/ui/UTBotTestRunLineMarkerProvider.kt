package com.huawei.utbot.cpp.ui

import com.huawei.utbot.cpp.actions.RunWithCoverageAction
import com.huawei.utbot.cpp.services.TestsResultsStorage
import com.intellij.codeInsight.daemon.LineMarkerInfo
import com.intellij.codeInsight.daemon.LineMarkerProvider
import com.intellij.openapi.actionSystem.AnAction
import com.intellij.openapi.components.service
import com.intellij.openapi.diagnostic.Logger
import com.intellij.openapi.editor.markup.GutterIconRenderer
import com.intellij.psi.PsiElement
import javax.swing.Icon

class UTBotTestRunLineMarkerProvider : LineMarkerProvider {
    val log = Logger.getInstance(this::class.java)
    private class UTBotRunWithCoverageLineMarkerInfo(callElement: PsiElement, message: String, icon: Icon) :
        LineMarkerInfo<PsiElement>(
            callElement,
            callElement.textRange,
            icon,
            { message },
            null,
            GutterIconRenderer.Alignment.LEFT,
            { message }
        ) {
        val log = Logger.getInstance(this::class.java)
        override fun createGutterRenderer(): GutterIconRenderer {
            return object : LineMarkerInfo.LineMarkerGutterIconRenderer<PsiElement>(this) {
                override fun isNavigateAction(): Boolean = true
                override fun getClickAction(): AnAction? {
                    log.debug("getClickAction was called!")
                    return element?.let { RunWithCoverageAction(it) }
                }
            }
        }
    }

    override fun getLineMarkerInfo(element: PsiElement): LineMarkerInfo<*>? {
        if (element.firstChild != null || (element.text != "TEST" && element.text != "UTBot")) {
            return null
        }
        if (element.containingFile.name.endsWith(".h")) return null
        val message = if (element.text == "TEST") "UTBot: Run with coverage" else "Run all tests with coverage"
        val icon = element.project.service<TestsResultsStorage>().getTestStatusIcon(element)
        return UTBotRunWithCoverageLineMarkerInfo(element, message, icon)
    }
}