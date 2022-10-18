package org.utbot.cpp.clion.plugin.tests.buildingRequestsTests

import org.junit.jupiter.api.Assertions
import org.junit.jupiter.api.Test
import org.utbot.cpp.clion.plugin.grpc.RemoteMapping
import org.utbot.cpp.clion.plugin.settings.UTBotProjectStoredSettings
import org.utbot.cpp.clion.plugin.utils.getBuilderForFileRequest
import testsgen.Testgen

class FileRequestBuildingTest : BaseBuildingTest() {
    private fun doTest(
        testedFileRelativePath: String,
        remotePath: String = UTBotProjectStoredSettings.REMOTE_PATH_VALUE_FOR_LOCAL_SCENARIO
    ) {
        setRemoteScenarioInPlugin(remotePath)
        val isRemoteExpected = isRemoteScenarioExpectedInTests(remotePath)

        val actualRequest: Testgen.FileRequest =
            createActionEventFromEditor(testedFileRelativePath).getBuilderForFileRequest().build(RemoteMapping(project))

        val expectedRequest: Testgen.FileRequest = Testgen.FileRequest.newBuilder().apply {
            projectRequest = Testgen.ProjectRequest.newBuilder().apply {
                targetPath = settings.targetPath
                synchronizeCode = isRemoteExpected
                projectContext = this@FileRequestBuildingTest.createExpectedProjectContext(remotePath)
                settingsContext = this@FileRequestBuildingTest.createExpectedSettingsContextFromCurrentSettings()
            }.build()
            filePath = if (isRemoteExpected) "$remotePath/$testedFileRelativePath" else projectPath.resolve(
                testedFileRelativePath
            ).toString()
        }.build()

        Assertions.assertEquals(expectedRequest, actualRequest)
    }

    @Test
    fun `test generateForFile grpc request is built correctly from action event in local scenario`() {
        doTest("lib/basic_functions.c")
    }

    @Test
    fun `test grpc FileRequest is built correctly from action event in remote scenario`() {
        doTest("lib/basic_functions.c", "/some/remote/path/${project.name}")
    }
}
