package org.utbot.cpp.clion.plugin.tests.buildingRequestsTests

import org.junit.jupiter.api.Assertions
import org.junit.jupiter.api.Test
import org.utbot.cpp.clion.plugin.grpc.RemoteMapping
import org.utbot.cpp.clion.plugin.settings.UTBotProjectStoredSettings
import org.utbot.cpp.clion.plugin.tests.integrationTests.GenerateForLineTest
import org.utbot.cpp.clion.plugin.utils.getBuilderForLineRequest
import testsgen.Testgen
import testsgen.Util

class LineRequestBuildingTest : BaseBuildingTest() {
    private fun doTest(
        testedFileRelativeFilePath: String,
        lineNumber: Int, // 1-indexed
        remotePath: String = UTBotProjectStoredSettings.REMOTE_PATH_VALUE_FOR_LOCAL_SCENARIO
    ) {
        this.setRemoteScenarioInPlugin(remotePath)
        val isRemoteExpected = this.isRemoteScenarioExpectedInTests(remotePath)

        val actualRequest: Testgen.LineRequest =
            this.createActionEventFromEditor(testedFileRelativeFilePath, lineNumber)
                .getBuilderForLineRequest().build(RemoteMapping(project))

        val expectedRequest: Testgen.LineRequest = Testgen.LineRequest.newBuilder().apply {
            projectRequest = Testgen.ProjectRequest.newBuilder().apply {
                targetPath = settings.targetPath
                synchronizeCode = isRemoteExpected
                projectContext = createExpectedProjectContext(remotePath)
                settingsContext = createExpectedSettingsContextFromCurrentSettings()
            }.build()
            sourceInfo = Util.SourceInfo.newBuilder().apply {
                filePath = if (isRemoteExpected) "$remotePath/$testedFileRelativeFilePath" else projectPath.resolve(
                    testedFileRelativeFilePath
                ).toString()
                line = lineNumber
            }.build()
        }.build()

        Assertions.assertEquals(expectedRequest, actualRequest)
    }

    @Test
    fun `test grpc LineRequest is built correctly from action event in local scenario`() {
        doTest("lib/basic_functions.c", GenerateForLineTest.IF_IN_MAX_FUNCTION_LINE)
    }

    @Test
    fun `test grpc LineRequest is built correctly from action event in remote scenario`() {
        doTest(
            "lib/basic_functions.c",
            GenerateForLineTest.IF_IN_MAX_FUNCTION_LINE,
            "/some/remote/path/${project.name}"
        )
    }
}
