package com.huawei.utbot.cpp

import com.huawei.utbot.cpp.actions.GenerateForFileAction
import com.huawei.utbot.cpp.client.Client
import com.huawei.utbot.cpp.services.UTBotSettings
import com.intellij.openapi.actionSystem.ActionManager
import com.intellij.openapi.components.service
import com.intellij.openapi.project.Project
import com.intellij.testFramework.UsefulTestCase
import com.intellij.testFramework.fixtures.CodeInsightTestFixture
import com.intellij.testFramework.fixtures.IdeaTestFixtureFactory
import kotlinx.coroutines.delay
import kotlinx.coroutines.job
import kotlinx.coroutines.runBlocking
import java.io.File
import java.nio.file.Paths

class GenerateForFileTest : UsefulTestCase() {
    val mountedProjectPath = "../../../bindMounts/f/c-example2"
    val realProjectPath = "../integration-tests/c-example"
    val testProjectpath = Paths.get(File(".").canonicalPath).resolve(mountedProjectPath).normalize()
    val testProjectName = "c-example2"
    val testProjectTestDir = testProjectpath.resolve("cl-plugin-test-tests")
    val testProjectBuildDir = testProjectpath.resolve("cl-plugin-test-buildDir")

    val myFixture: CodeInsightTestFixture = createFixture()
    val project: Project = myFixture.project
    val settings: UTBotSettings = project.service()
    val client: Client = project.service()

    fun createFixture(): CodeInsightTestFixture {
        val fixture = IdeaTestFixtureFactory.getFixtureFactory().let {
            it.createCodeInsightFixture(it.createFixtureBuilder(testProjectName, testProjectpath, false).fixture)
        }
        fixture.setUp()
        fixture.testDataPath = testProjectpath.toString()
        return fixture
    }

    init {
        println("$settings")
    }

    // called before each test
    override fun setUp() {
        settings.buildDirPath = testProjectBuildDir.toString()
        settings.testDirPath = testProjectTestDir.toString()
        settings.remotePath = "/home/utbot/projects/f/c-example2"
    }

    fun waitForRequestsToFinish() {
        runBlocking {
            while (client.shortLivingRequestsCS.coroutineContext.job.children.toList().isNotEmpty()) {
                delay(500L)
            }
        }
    }

    fun testGenerateForFile() {
        val id = ActionManager.getInstance().getId(GenerateForFileAction())
        client.createBuildDir()
        client.generateJSon()

        // requests to server are asynchronous, need to wait for server to respond
        waitForRequestsToFinish()

        myFixture.performEditorAction(id)
    }

    // called after each test
    override fun tearDown() {
        // somehow project service Client is not disposed automatically by the ide, and the exception is thrown that
        // timer related to heartbeat is not disposed. So let's dispose it manually.
        client.dispose()
    }
}
