package org.utbot.cpp.clion.plugin.grpc

import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.CommonDataKeys
import com.intellij.openapi.project.Project
import org.utbot.cpp.clion.plugin.settings.settings
import org.utbot.cpp.clion.plugin.utils.activeProject
import org.utbot.cpp.clion.plugin.utils.convertToRemotePathIfNeeded
import testsgen.Testgen
import testsgen.Util

fun getProjectGrpcRequest(e: AnActionEvent): Testgen.ProjectRequest = getProjectGrpcRequest(e.activeProject())

fun getFolderGrpcRequest(e: AnActionEvent): Testgen.FolderRequest {
    val project = e.activeProject()
    val localPath = e.getRequiredData(CommonDataKeys.VIRTUAL_FILE).path

    return Testgen.FolderRequest.newBuilder()
        .setProjectRequest(getProjectGrpcRequest(e))
        .setFolderPath(localPath.convertToRemotePathIfNeeded(project))
        .build()
}

fun getFileGrpcRequest(e: AnActionEvent): Testgen.FileRequest {
    val project = e.activeProject()
    val filePath = e.getRequiredData(CommonDataKeys.VIRTUAL_FILE).path

    return Testgen.FileRequest.newBuilder()
        .setProjectRequest(getProjectGrpcRequest(project))
        .setFilePath(filePath.convertToRemotePathIfNeeded(project))
        .build()
}

fun getClassGrpcRequest(e: AnActionEvent): Testgen.ClassRequest = Testgen.ClassRequest.newBuilder()
    .setLineRequest(getLineGrpcRequest(e))
    .build()

fun getFunctionGrpcRequest(e: AnActionEvent): Testgen.FunctionRequest {
    val lineRequest = getLineGrpcRequest(e)
    return Testgen.FunctionRequest.newBuilder()
        .setLineRequest(lineRequest)
        .build()
}

fun getSnippetGrpcRequest(e: AnActionEvent): Testgen.SnippetRequest {
    val project = e.activeProject()
    val localPath = e.getRequiredData(CommonDataKeys.VIRTUAL_FILE).path

    return Testgen.SnippetRequest.newBuilder()
        .setProjectContext(getProjectContextMessage(project))
        .setSettingsContext(getSettingsContextMessage(project))
        .setFilePath(localPath.convertToRemotePathIfNeeded(project))
        .build()
}

fun getLineGrpcRequest(e: AnActionEvent): Testgen.LineRequest {
    val project = e.getRequiredData(CommonDataKeys.PROJECT)
    val filePath = e.getRequiredData(CommonDataKeys.VIRTUAL_FILE).path
    val editor = e.getRequiredData(CommonDataKeys.EDITOR)
    val lineNumber = editor.caretModel.logicalPosition.line + 1

    return getLineGrpcRequest(project, lineNumber, filePath)
}

fun getAssertionGrpcRequest(e: AnActionEvent): Testgen.AssertionRequest {
    return Testgen.AssertionRequest.newBuilder()
        .setLineRequest(getLineGrpcRequest(e))
        .build()
}

fun getPredicateGrpcRequest(
    e: AnActionEvent,
    predicate: String,
    validationType: Util.ValidationType,
    returnValue: String,
): Testgen.PredicateRequest {
    val predicateInfo = getPredicateGrpcRequest(predicate, returnValue, validationType)
    return Testgen.PredicateRequest.newBuilder()
        .setLineRequest(getLineGrpcRequest(e))
        .setPredicateInfo(predicateInfo)
        .build()
}

private fun getPredicateGrpcRequest(predicate: String, returnValue: String, type: Util.ValidationType): Util.PredicateInfo =
    Util.PredicateInfo.newBuilder()
        .setPredicate(predicate)
        .setReturnValue(returnValue)
        .setType(type)
        .build()

private fun getProjectGrpcRequest(project: Project): Testgen.ProjectRequest {
    return Testgen.ProjectRequest.newBuilder()
        .setSettingsContext(getSettingsContextMessage(project))
        .setProjectContext(getProjectContextMessage(project))
        .setTargetPath(project.settings.convertedTargetPath)
        .addAllSourcePaths(project.settings.convertedSourcePaths)
        .setSynchronizeCode(project.settings.isRemoteScenario)
        .build()
}


private fun getLineGrpcRequest(project: Project, line: Int, filePath: String): Testgen.LineRequest =
    Testgen.LineRequest.newBuilder()
        .setProjectRequest(getProjectGrpcRequest(project))
        .setSourceInfo(getSourceInfo(project, line, filePath))
        .build()

private fun getSourceInfo(project: Project, line: Int, filePath: String): Util.SourceInfo =
    Util.SourceInfo.newBuilder()
        .setLine(line)
        .setFilePath(filePath.convertToRemotePathIfNeeded(project))
        .build()
