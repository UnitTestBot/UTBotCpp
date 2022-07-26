package org.utbot.cpp.clion.plugin.grpc

import com.intellij.openapi.project.Project
import org.utbot.cpp.clion.plugin.utils.convertToRemotePathIfNeeded
import testsgen.Testgen

/**
 * [testName] and [testSuiteName] are non-empty if a concrete test is specified.
 * Request for several tests leaves these fields empty.
 */
fun getCoverageAndResultsGrpcRequest(
    project: Project,
    filePath: String,
    testSuiteName: String = "",
    testName: String = "",
    includeCoverage: Boolean = true,
): Testgen.CoverageAndResultsRequest {
    val remoteFilePath = filePath.convertToRemotePathIfNeeded(project)

    return Testgen.CoverageAndResultsRequest.newBuilder()
        .setCoverage(includeCoverage)
        .setProjectContext(getProjectContextMessage(project))
        .setSettingsContext(getSettingsContextMessage(project))
        .setTestFilter(getTestFilter(remoteFilePath, testName, testSuiteName))
        .build()
}

private fun getTestFilter(filePath: String, testName: String = "", testSuiteName: String = ""): Testgen.TestFilter =
    Testgen.TestFilter.newBuilder()
        .setTestFilePath(filePath)
        .setTestName(testName)
        .setTestSuite(testSuiteName)
        .build()
