package org.utbot.cpp.clion.plugin.grpc

import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.CommonDataKeys
import com.intellij.openapi.project.Project
import org.utbot.cpp.clion.plugin.utils.convertToRemotePathIfNeeded
import testsgen.Testgen
import testsgen.Util

fun getProjectRequest(e: AnActionEvent): Testgen.ProjectRequest = getProjectRequest(e.activeProject())

fun getFolderRequest(e: AnActionEvent): Testgen.FolderRequest {
    val project = e.activeProject()
    val localPath = e.getRequiredData(CommonDataKeys.VIRTUAL_FILE).path

    return Testgen.FolderRequest.newBuilder()
        .setProjectRequest(getProjectRequest(e))
        .setFolderPath(localPath.convertToRemotePathIfNeeded(project))
        .build()
}

fun getFileRequest(e: AnActionEvent): Testgen.FileRequest {
    val project = e.activeProject()
    val filePath = e.getRequiredData(CommonDataKeys.VIRTUAL_FILE).path

    return Testgen.FileRequest.newBuilder()
        .setProjectRequest(getProjectRequest(project))
        .setFilePath(filePath.convertToRemotePathIfNeeded(project))
        .build()
}

fun getClassRequest(e: AnActionEvent): Testgen.ClassRequest = Testgen.ClassRequest.newBuilder()
    .setLineRequest(getLineRequest(e))
    .build()

fun getFunctionRequest(e: AnActionEvent): Testgen.FunctionRequest {
    val lineRequest = getLineRequest(e)
    return Testgen.FunctionRequest.newBuilder()
        .setLineRequest(lineRequest)
        .build()
}

fun getSnippetRequest(e: AnActionEvent): Testgen.SnippetRequest {
    val project = e.activeProject()
    val localPath = e.getRequiredData(CommonDataKeys.VIRTUAL_FILE).path

    return Testgen.SnippetRequest.newBuilder()
        .setProjectContext(getProjectContextMessage(project))
        .setSettingsContext(getSettingsContextMessage(project))
        .setFilePath(localPath.convertToRemotePathIfNeeded(project))
        .build()
}

fun getLineRequest(e: AnActionEvent): Testgen.LineRequest {
    val project = e.getRequiredData(CommonDataKeys.PROJECT)
    val filePath = e.getRequiredData(CommonDataKeys.VIRTUAL_FILE).path
    val editor = e.getRequiredData(CommonDataKeys.EDITOR)
    val lineNumber = editor.caretModel.logicalPosition.line + 1

    return getLineRequest(project, lineNumber, filePath)
}

fun getAssertionRequest(e: AnActionEvent): Testgen.AssertionRequest {
    return Testgen.AssertionRequest.newBuilder()
        .setLineRequest(getLineRequest(e))
        .build()
}

fun getPredicateRequest(
    e: AnActionEvent,
    predicate: String,
    validationType: Util.ValidationType,
    returnValue: String,
): Testgen.PredicateRequest {
    val predicateInfo = getPredicateRequest(predicate, returnValue, validationType)
    return Testgen.PredicateRequest.newBuilder()
        .setLineRequest(getLineRequest(e))
        .setPredicateInfo(predicateInfo)
        .build()
}

private fun getPredicateRequest(predicate: String, returnValue: String, type: Util.ValidationType): Util.PredicateInfo =
    Util.PredicateInfo.newBuilder()
        .setPredicate(predicate)
        .setReturnValue(returnValue)
        .setType(type)
        .build()

private fun getProjectRequest(project: Project): Testgen.ProjectRequest {
    val settings = project.allSettings()
    return Testgen.ProjectRequest.newBuilder()
        .setSettingsContext(getSettingsContextMessage(project))
        .setProjectContext(getProjectContextMessage(project))
        .setTargetPath(settings.convertedTargetPath)
        .addAllSourcePaths(settings.convertedSourcePaths)
        .setSynchronizeCode(settings.isRemoteScenario)
        .build()
}


private fun getLineRequest(project: Project, line: Int, filePath: String): Testgen.LineRequest =
    Testgen.LineRequest.newBuilder()
        .setProjectRequest(getProjectRequest(project))
        .setSourceInfo(getSourceInfo(project, line, filePath))
        .build()

private fun getSourceInfo(project: Project, line: Int, filePath: String): Util.SourceInfo =
    Util.SourceInfo.newBuilder()
        .setLine(line)
        .setFilePath(filePath.convertToRemotePathIfNeeded(project))
        .build()
