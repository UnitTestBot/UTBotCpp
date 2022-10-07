package org.utbot.cpp.clion.plugin.tests.buildingRequestsTests

import org.junit.jupiter.api.Assertions
import org.junit.jupiter.api.Test
import org.utbot.cpp.clion.plugin.grpc.GrpcRequestBuilderFactory
import org.utbot.cpp.clion.plugin.grpc.RemoteMapping
import org.utbot.cpp.clion.plugin.settings.UTBotProjectStoredSettings
import testsgen.Testgen

class ProjectRequestBuildingTest : BaseBuildingTest() {
    private fun doTest(remotePath: String = UTBotProjectStoredSettings.REMOTE_PATH_VALUE_FOR_LOCAL_SCENARIO) {
        setRemoteScenarioInPlugin(remotePath)
        val isRemoteExpected = isRemoteScenarioExpectedInTests(remotePath)

        val actualRequest: Testgen.ProjectRequest =
            GrpcRequestBuilderFactory(project).createProjectRequestBuilder().build(RemoteMapping(project))

        val expectedRequest: Testgen.ProjectRequest = Testgen.ProjectRequest.newBuilder().apply {
            targetPath = settings.targetPath
            synchronizeCode = isRemoteExpected
            projectContext = createExpectedProjectContext(remotePath)
            settingsContext = createExpectedSettingsContextFromCurrentSettings()
        }.build()

        Assertions.assertEquals(expectedRequest, actualRequest)
    }

    @Test
    fun `test grpc ProjectRequest is built correctly in local scenario`() {
        doTest(UTBotProjectStoredSettings.REMOTE_PATH_VALUE_FOR_LOCAL_SCENARIO)
    }

    @Test
    fun `test grpc ProjectRequest is built correctly in remote scenario`() {
        doTest("/some/remote/path")
    }
}
