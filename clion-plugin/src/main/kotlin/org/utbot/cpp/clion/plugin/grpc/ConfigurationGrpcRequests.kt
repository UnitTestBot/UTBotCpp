package org.utbot.cpp.clion.plugin.grpc

import com.intellij.openapi.project.Project
import com.jetbrains.cidr.cpp.cmake.workspace.CMakeWorkspace
import testsgen.Testgen

fun getProjectTargetsGrpcRequest(project: Project): Testgen.ProjectTargetsRequest =
    Testgen.ProjectTargetsRequest.newBuilder()
        .setProjectContext(getProjectContextMessage(project))
        .build()

fun getProjectConfigGrpcRequest(project: Project, configMode: Testgen.ConfigMode): Testgen.ProjectConfigRequest {
    val builder = Testgen.ProjectConfigRequest.newBuilder()
        .setProjectContext(getProjectContextMessage(project))
        .setConfigMode(configMode)
    getCmakeOptions(project)?.let { builder.setCmakeOptions(0, it) }

    return builder.build()
}

fun getDummyGrpcRequest(): Testgen.DummyRequest = Testgen.DummyRequest.newBuilder().build()

fun getLogChannelGrpcRequest(logLevel: String): Testgen.LogChannelRequest =
    Testgen.LogChannelRequest.newBuilder()
        .setLogLevel(logLevel)
        .build()

private fun getCmakeOptions(project: Project): String? =
    CMakeWorkspace.getInstance(project).profileInfos
        .map { it.profile }
        .firstOrNull { it.enabled }
        ?.generationOptions