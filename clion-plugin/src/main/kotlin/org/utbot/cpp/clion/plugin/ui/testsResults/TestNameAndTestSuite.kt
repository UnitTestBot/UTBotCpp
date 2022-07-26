package org.utbot.cpp.clion.plugin.ui.testsResults

import com.intellij.psi.PsiDocumentManager
import com.intellij.psi.PsiElement
import com.intellij.util.DocumentUtil

data class TestNameAndTestSuite(val name: String = "", val suite: String = "") {
    companion object {
        fun getFromPsiElement(element: PsiElement): TestNameAndTestSuite {
            val document = PsiDocumentManager.getInstance(element.project).getDocument(element.containingFile) ?: error(
                "Could not get document"
            )
            val startOffset = DocumentUtil.getLineStartOffset(element.textOffset, document)
            val endOffset = DocumentUtil.getLineEndOffset(element.textOffset, document)

            val lineText = document.text.substring(startOffset..endOffset)
            val testArgs = """\((.+)\)""".toRegex().find(lineText)?.groupValues?.getOrNull(1)?.let {
                """([^\s,]+),\s*([^\s,]+)""".toRegex().find(it)?.destructured
            }
            val suiteName = testArgs?.component1() ?: ""
            val testedMethodName = testArgs?.component2() ?: ""
            return TestNameAndTestSuite(testedMethodName, suiteName)
        }
    }
}