package com.huawei.utbot.cpp

import com.huawei.utbot.cpp.client.Client
import com.huawei.utbot.cpp.services.UTBotSettings
import com.intellij.openapi.components.service
import com.intellij.openapi.project.Project
import com.intellij.testFramework.UsefulTestCase
import com.intellij.testFramework.fixtures.CodeInsightTestFixture
import com.intellij.testFramework.fixtures.IdeaTestFixtureFactory
import com.intellij.testFramework.fixtures.impl.TempDirTestFixtureImpl
import com.intellij.util.io.delete
import kotlinx.coroutines.delay
import kotlinx.coroutines.job
import kotlinx.coroutines.runBlocking
import java.io.File
import java.nio.file.Path
import java.nio.file.Paths

/**
 * Implementation of TempDirTestFixture that uses [testsDirectory] as
 * a tempDirectory, and does not delete it on tearDown.
 *
 * Intellij Platform tests are based on files in temp directory, which is provided and managed by TempDirTestFixture.
 * On tearDown, temp directory is deleted.
 * The problem with temp directory that it is not mounted to docker as it is generated
 * each time setUp is called. So if server is inside docker container the project lying in
 * temp directory can't be accessed by server. This class solves the problem, by using [testsDirectory]
 * instead of some generated temp directory.
 */
class TestFixtureProxy(val testsDirectory: Path): TempDirTestFixtureImpl() {
    override fun doCreateTempDirectory(): Path {
        return testsDirectory
    }

    // as the directory is not actually temporary, it should not be deleted
    override fun deleteOnTearDown() = false
}

class GenerateForFileTest : UsefulTestCase() {
    // val mountedProjectPath = "../../../bindMounts/f/c-example2"

    private val realProjectPath = "../integration-tests/c-example"
    private val testProjectPath = Paths.get(File(".").canonicalPath).resolve(realProjectPath).normalize()
    private val testProjectName = "c-example2"
    private val testProjectTestDir = testProjectPath.resolve("cl-plugin-test-tests")
    private val testProjectBuildDir = testProjectPath.resolve("cl-plugin-test-buildDir")

    private val myFixture: CodeInsightTestFixture = createFixture()
    private val project: Project = myFixture.project
    private val settings: UTBotSettings = project.service()
    private val client: Client = project.service()

    private fun createFixture(): CodeInsightTestFixture {
        val fixture = IdeaTestFixtureFactory.getFixtureFactory().let {
            it.createCodeInsightFixture(it.createFixtureBuilder(testProjectName, testProjectPath, false).fixture, TestFixtureProxy(testProjectPath))
        }
        fixture.setUp()
        fixture.testDataPath = testProjectPath.toString()
        return fixture
    }

    // called before each test
    override fun setUp() {
        settings.buildDirPath = testProjectBuildDir.toString()
        settings.testDirPath = testProjectTestDir.toString()
        settings.remotePath = "/home/utbot/projects/f/c-example2"
        buildProject()
    }

    // requests to server are asynchronous, need to wait for server to respond
    fun waitForRequestsToFinish() {
        runBlocking {
            while (client.shortLivingRequestsCS.coroutineContext.job.children.toList().isNotEmpty()) {
                delay(500L)
            }
        }
    }


    fun testGenerateForFile() {
        myFixture.configureFromTempProjectFile("/lib/basic_functions.c")
        myFixture.performEditorAction("com.huawei.utbot.cpp.actions.GenerateForFileActionInEditor")
        waitForRequestsToFinish()
        checkFileExists(testProjectBuildDir, "build dir does not exist")
        checkFileExists(testProjectTestDir, "tests folder does not exist")
        checkFileExists(testProjectTestDir.resolve("lib/basic_functions_test.cpp"), "generated test file does not exist ")
    }

    // called after each test
    override fun tearDown() {
        // somehow project service Client is not disposed automatically by the ide, and the exception is thrown that
        // timer related to heartbeat is not disposed. So let's dispose it manually.
        client.dispose()
        testProjectBuildDir.delete(recursively = true)
        testProjectTestDir.delete(recursively = true)
    }
}
