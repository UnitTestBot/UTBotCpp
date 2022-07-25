package org.utbot.cpp.clion.plugin.grpc

import com.intellij.openapi.components.service
import com.intellij.openapi.project.Project
import org.utbot.cpp.clion.plugin.settings.UTBotAllSettings
import testsgen.Testgen

fun getSettingsContextMessage(project: Project): Testgen.SettingsContext {
    val settings = project.allSettings()
    return Testgen.SettingsContext.newBuilder()
        .setVerbose(settings.verbose)
        .setUseStubs(settings.useStubs)
        .setTimeoutPerTest(settings.timeoutPerTest)
        .setTimeoutPerFunction(settings.timeoutPerFunction)
        .setGenerateForStaticFunctions(settings.generateForStaticFunctions)
        .setUseDeterministicSearcher(settings.useDeterministicSearcher)
        .build()
}

fun getProjectContextMessage(project: Project): Testgen.ProjectContext {
    val settings = project.allSettings()
    return Testgen.ProjectContext.newBuilder()
        .setProjectName(project.name)
        .setProjectPath(settings.convertedProjectPath)
        .setBuildDirRelativePath(settings.buildDirRelativePath)
        .setResultsDirRelativePath("") // this path is used only by command line interface, server doesn't require it.
        .setTestDirPath(settings.convertedTestDirPath)
        .build()
}

fun Project.allSettings() = this.service<UTBotAllSettings>()