package org.utbot.cpp.clion.plugin.grpc

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
    .setTestDirPath(project.settings.convertedTestDirPath)
    .build()