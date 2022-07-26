package org.utbot.cpp.clion.plugin.utils

import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.CommonDataKeys
import com.intellij.openapi.components.service
import com.intellij.openapi.project.Project
import com.jetbrains.cidr.cpp.cmake.workspace.CMakeWorkspace
import org.utbot.cpp.clion.plugin.settings.UTBotAllSettings
import testsgen.Testgen
import testsgen.Util

fun getSettingsContextMessage(params: UTBotAllSettings): Testgen.SettingsContext {
    return Testgen.SettingsContext.newBuilder()
        .setVerbose(params.verbose)
        .setUseStubs(params.useStubs)
        .setTimeoutPerTest(params.timeoutPerTest)
        .setTimeoutPerFunction(params.timeoutPerFunction)
        .setGenerateForStaticFunctions(params.generateForStaticFunctions)
        .setUseDeterministicSearcher(params.useDeterministicSearcher)
        .build()
}

fun getProjectContextMessage(params: UTBotAllSettings, project: Project): Testgen.ProjectContext {
    return Testgen.ProjectContext.newBuilder()
        .setProjectName(project.name)
        .setProjectPath(params.convertedProjectPath)
        .setBuildDirRelativePath(params.buildDirRelativePath)
        .setResultsDirRelativePath("") // this path is used only for console interface, server don't use it.
        .setTestDirPath(params.convertedTestDirPath)
        .build()
}

fun getProjectContextMessage(e: AnActionEvent): Testgen.ProjectContext {
    return getProjectContextMessage(e.project?.service()!!, e.project!!)
}

fun getProjectRequestMessage(project: Project, params: UTBotAllSettings): Testgen.ProjectRequest {
    return Testgen.ProjectRequest.newBuilder()
        .setSettingsContext(
            getSettingsContextMessage(
                project.service()
            )
        )
        .setProjectContext(getProjectContextMessage(params, project))
        .setTargetPath(params.convertedTargetPath)
        .addAllSourcePaths(params.convertedSourcePaths)
        .setSynchronizeCode(project.utbotSettings.isRemoteScenario())
        .build()
}

fun getSourceInfoMessage(line: Int, filePath: String, project: Project): Util.SourceInfo {
    return Util.SourceInfo.newBuilder()
        .setLine(line)
        .setFilePath(filePath.convertToRemotePathIfNeeded(project))
        .build()
}

fun getLineRequestMessage(project: Project, params: UTBotAllSettings, line: Int, filePath: String): Testgen.LineRequest {
    val projectRequest = getProjectRequestMessage(project, params)
    val sourceInfo = getSourceInfoMessage(line, filePath, project)
    return Testgen.LineRequest.newBuilder()
        .setProjectRequest(projectRequest)
        .setSourceInfo(sourceInfo)
        .build()
}

fun getLineRequestMessage(e: AnActionEvent): Testgen.LineRequest {
    val project = e.getRequiredData(CommonDataKeys.PROJECT)
    val filePath = e.getRequiredData(CommonDataKeys.VIRTUAL_FILE).path
    val editor = e.getRequiredData(CommonDataKeys.EDITOR)
    val utbotSettings = project.utbotSettings
    val lineNumber = editor.caretModel.logicalPosition.line + 1
    return getLineRequestMessage(project, utbotSettings, lineNumber, filePath)
}

fun getFunctionRequestMessage(e: AnActionEvent): Testgen.FunctionRequest {
    val lineRequest = getLineRequestMessage(e)
    return Testgen.FunctionRequest.newBuilder()
        .setLineRequest(lineRequest)
        .build()
}

fun getProjectRequestMessage(e: AnActionEvent): Testgen.ProjectRequest {
    return getProjectRequestMessage(e.project!!, e.project!!.service())
}

fun getFileRequestMessage(e: AnActionEvent): Testgen.FileRequest {
    // this function is supposed to be called in actions' performAction(), so update() validated these properties
    val project: Project = e.project!!
    val utbotSettings = project.utbotSettings
    val filePath = e.getRequiredData(CommonDataKeys.VIRTUAL_FILE).path
    return Testgen.FileRequest.newBuilder()
        .setProjectRequest(getProjectRequestMessage(project, utbotSettings))
        .setFilePath(filePath.convertToRemotePathIfNeeded(project))
        .build()
}

fun getPredicateInfoMessage(predicate: String, returnValue: String, type: Util.ValidationType): Util.PredicateInfo {
    return Util.PredicateInfo.newBuilder()
        .setPredicate(predicate)
        .setReturnValue(returnValue)
        .setType(type)
        .build()
}

fun getClassRequestMessage(e: AnActionEvent): Testgen.ClassRequest {
    return Testgen.ClassRequest.newBuilder().setLineRequest(
        getLineRequestMessage(e)
    ).build()
}

fun getFolderRequestMessage(e: AnActionEvent): Testgen.FolderRequest {
    val localPath = e.getRequiredData(CommonDataKeys.VIRTUAL_FILE).path
    return Testgen.FolderRequest.newBuilder()
        .setProjectRequest(getProjectRequestMessage(e))
        .setFolderPath(localPath.convertToRemotePathIfNeeded(e.project!!))
        .build()
}

fun getSnippetRequestMessage(e: AnActionEvent): Testgen.SnippetRequest {
    val localPath = e.getRequiredData(CommonDataKeys.VIRTUAL_FILE).path
    return Testgen.SnippetRequest.newBuilder()
        .setProjectContext(getProjectContextMessage(e))
        .setSettingsContext(getSettingsContextMessage(e.project!!.service()))
        .setFilePath(localPath.convertToRemotePathIfNeeded(e.project!!))
        .build()
}

fun getAssertionRequestMessage(e: AnActionEvent): Testgen.AssertionRequest {
    return Testgen.AssertionRequest.newBuilder()
        .setLineRequest(getLineRequestMessage(e))
        .build()
}

fun getPredicateRequestMessage(
    validationType: Util.ValidationType, returnValue: String, predicate: String,
    e: AnActionEvent
): Testgen.PredicateRequest {
    val predicateInfo = getPredicateInfoMessage(predicate, returnValue, validationType)
    return Testgen.PredicateRequest.newBuilder()
        .setLineRequest(getLineRequestMessage(e))
        .setPredicateInfo(predicateInfo)
        .build()
}

fun getProjectConfigRequestMessage(project: Project, configMode: Testgen.ConfigMode): Testgen.ProjectConfigRequest {
    val builder = Testgen.ProjectConfigRequest.newBuilder()
        .setProjectContext(getProjectContextMessage(project.service(), project))
        .setConfigMode(configMode)

    getCmakeOptions(project)?.let {
        builder.setCmakeOptions(0, it)
    }

    return builder.build()
}

fun getCmakeOptions(project: Project): String? {
    return CMakeWorkspace.getInstance(project).profileInfos.find {
        it.profile.enabled
    }?.profile?.generationOptions
}

fun getDummyRequest(): Testgen.DummyRequest = Testgen.DummyRequest.newBuilder().build()

fun getLogChannelRequest(logLevel: String): Testgen.LogChannelRequest =
    Testgen.LogChannelRequest.newBuilder().setLogLevel(logLevel).build()

fun getTestFilter(e: AnActionEvent): Testgen.TestFilter {
    val filePath = e.getRequiredData(CommonDataKeys.VIRTUAL_FILE).path
    val testName = ""
    val testSuite = ""
    return getTestFilter(filePath, testName, testSuite)
}

fun getTestFilter(filePath: String, testName: String = "", testSuite: String = ""): Testgen.TestFilter =
    Testgen.TestFilter.newBuilder()
        .setTestFilePath(filePath)
        .setTestName(testName)
        .setTestSuite(testSuite)
        .build()

fun getCoverageAndResultsRequest(
    utbotSettings: UTBotAllSettings,
    filePath: String,
    testSuite: String = "",
    testName: String = "",
    includeCoverage: Boolean = true
): Testgen.CoverageAndResultsRequest {
    val remoteFilePath = filePath.convertToRemotePathIfNeeded(utbotSettings.project)
    return Testgen.CoverageAndResultsRequest.newBuilder()
        .setCoverage(includeCoverage)
        .setProjectContext(getProjectContextMessage(utbotSettings, utbotSettings.project))
        .setSettingsContext(getSettingsContextMessage(utbotSettings.project.service()))
        .setTestFilter(getTestFilter(remoteFilePath, testName, testSuite))
        .build()
}

fun getCoverageAndResultsRequest(
    e: AnActionEvent,
    suiteName: String = "",
    testName: String = "",
    includeCoverage: Boolean = true
): Testgen.CoverageAndResultsRequest {
    val utbotSettings = e.project!!.utbotSettings
    return getCoverageAndResultsRequest(
        utbotSettings,
        e.getRequiredData(CommonDataKeys.VIRTUAL_FILE).path,
        suiteName,
        testName,
        includeCoverage
    )
}

fun getProjectTargetsRequest(project: Project): Testgen.ProjectTargetsRequest {
    return Testgen.ProjectTargetsRequest.newBuilder()
        .setProjectContext(getProjectContextMessage(project.service(), project))
        .build()
}

fun getVersionInfo(): Testgen.VersionInfo = Testgen.VersionInfo.newBuilder().setVersion("2022.7").build()
