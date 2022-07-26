package org.utbot.cpp.clion.plugin.grpc

import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.project.Project
import org.utbot.cpp.clion.plugin.settings.settings
import testsgen.Testgen

fun getSettingsContextMessage(project: Project): Testgen.SettingsContext {
    val storedSettings = project.settings.storedSettings
    return Testgen.SettingsContext.newBuilder()
        .setVerbose(storedSettings.verbose)
        .setUseStubs(storedSettings.useStubs)
        .setTimeoutPerTest(storedSettings.timeoutPerTest)
        .setTimeoutPerFunction(storedSettings.timeoutPerFunction)
        .setGenerateForStaticFunctions(storedSettings.generateForStaticFunctions)
        .setUseDeterministicSearcher(storedSettings.useDeterministicSearcher)
        .build()
}

fun getProjectContextMessage(project: Project): Testgen.ProjectContext = Testgen.ProjectContext.newBuilder()
    .setProjectName(project.name)
    .setProjectPath(project.settings.convertedProjectPath)
    .setBuildDirRelativePath(project.settings.storedSettings.buildDirRelativePath)
    .setResultsDirRelativePath("") // this path is used only by command line interface, server doesn't require it.
    .setTestDirPath(project.settings.convertedTestDirPath)
    .build()

fun AnActionEvent.activeProject() = this.project
    ?: error("A project related to action event $this not found")