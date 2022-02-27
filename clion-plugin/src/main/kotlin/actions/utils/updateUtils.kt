package actions.utils

import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.CommonDataKeys
import com.intellij.psi.util.PsiTreeUtil
import com.jetbrains.cidr.lang.psi.OCFunctionDefinition
import com.jetbrains.cidr.lang.psi.OCStruct

fun getContainingFunction(e: AnActionEvent): OCFunctionDefinition? {
    val editor = e.getData(CommonDataKeys.EDITOR)
    val psiFile = e.getData(CommonDataKeys.PSI_FILE)
    val offset = editor?.caretModel?.offset ?: return null
    val element = psiFile?.findElementAt(offset)
    return PsiTreeUtil.getParentOfType(element, OCFunctionDefinition::class.java)
}

fun getContainingClass(e: AnActionEvent): OCStruct? {
    val editor = e.getData(CommonDataKeys.EDITOR)
    val psiFile = e.getData(CommonDataKeys.PSI_FILE)
    val offset = editor?.caretModel?.offset ?: return null
    val element = psiFile?.findElementAt(offset) ?: return null
    return PsiTreeUtil.getParentOfType(element, OCStruct::class.java)
}
