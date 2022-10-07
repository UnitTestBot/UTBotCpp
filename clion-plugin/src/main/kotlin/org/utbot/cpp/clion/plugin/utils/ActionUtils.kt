package org.utbot.cpp.clion.plugin.utils

import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.CommonDataKeys
import org.utbot.cpp.clion.plugin.grpc.GrpcRequestBuilder
import org.utbot.cpp.clion.plugin.grpc.GrpcRequestBuilderFactory
import org.utbot.cpp.clion.plugin.settings.settings
import testsgen.Testgen

fun AnActionEvent.getLineNumberUnsafe(): Int {
    val editor = this.getRequiredData(CommonDataKeys.EDITOR)
    return editor.caretModel.logicalPosition.line + 1
}

fun AnActionEvent.getFilePathUnsafe(): String {
    return getRequiredData(CommonDataKeys.VIRTUAL_FILE).localPath.toString()
}

fun AnActionEvent.getBuilderForFileRequest(): GrpcRequestBuilder<Testgen.FileRequest> {
    return GrpcRequestBuilderFactory(this.activeProject())
        .createFileRequestBuilder(this.getFilePathUnsafe())
}

fun AnActionEvent.getBuilderForLineRequest(): GrpcRequestBuilder<Testgen.LineRequest> {
    val filePath = this.getRequiredData(CommonDataKeys.VIRTUAL_FILE).path
    val editor = this.getRequiredData(CommonDataKeys.EDITOR)
    val lineNumber = editor.caretModel.logicalPosition.line + 1
    return GrpcRequestBuilderFactory(this.activeProject())
        .createLineRequestBuilder(lineNumber, filePath)
}

fun isPluginEnabled(e: AnActionEvent): Boolean {
    var isEnabled = false
    val project = e.project
    if (project != null) {
        isEnabled = project.settings.storedSettings.isPluginEnabled
    }
    return isEnabled
}