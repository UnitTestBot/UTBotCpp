package com.huawei.utbot.cpp

import com.huawei.utbot.cpp.client.Client
import com.huawei.utbot.cpp.services.UTBotSettings
import com.intellij.openapi.components.service
import com.intellij.testFramework.UsefulTestCase
import com.intellij.testFramework.fixtures.CodeInsightTestFixture
import com.intellij.testFramework.fixtures.IdeaTestFixtureFactory
import com.intellij.testFramework.fixtures.impl.TempDirTestFixtureImpl
import com.intellij.util.io.delete
import kotlinx.coroutines.delay
import kotlinx.coroutines.job
import kotlinx.coroutines.runBlocking
import java.io.BufferedReader
import java.io.File
import java.io.InputStreamReader
import java.nio.file.Files
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

class GenerateForFileTest(arg: Unit = println("default arg of GenerateForFileTest")) : UsefulTestCase() {
    init {
        println("init block of GenerateForFileTest")
    }
    private val mountedProjectPath = "../../../bindMounts/f/c-example2"
    private val realProjectPath = "../integration-tests/c-example"
    private val isMac = false

    private val relativeProjectPath = if (!isMac) realProjectPath else mountedProjectPath
    private val testProjectPath = Paths.get(File(".").canonicalPath).resolve(relativeProjectPath).normalize()
    private val testProjectName = Paths.get(relativeProjectPath).last().toString()
    private val testProjectTestDir = testProjectPath.resolve("cl-plugin-test-tests")
    private val testProjectBuildDir = testProjectPath.resolve("cl-plugin-test-buildDir")
    private val myFixture = createFixture()
    private val project = myFixture.project

    private fun createFixture(): CodeInsightTestFixture {
        println("Creating fixture")
        val fixture = IdeaTestFixtureFactory.getFixtureFactory().let {
            it.createCodeInsightFixture(it.createFixtureBuilder(testProjectName, testProjectPath, false).fixture, TestFixtureProxy(testProjectPath))
        }
        fixture.setUp()
        fixture.testDataPath = testProjectPath.toString()
        println("Finished creating fixture")
        return fixture
    }

    private val settings: UTBotSettings = project.service()
    private val client: Client = project.service()

    // called before each test
    override fun setUp() {
        println("setUP of my UsefulTestcase")
        super.setUp()
        settings.buildDirPath = testProjectBuildDir.toString()
        settings.testDirPath = testProjectTestDir.toString()
        if (isMac) {
            settings.remotePath = "/home/utbot/projects/f/c-example2"
            client.createBuildDir()
            client.generateJSon()
            waitForRequestsToFinish()
        } else {
            buildProject(buildDirName = testProjectBuildDir.last().toString())
        }
        println("setUp of my UsefulTestCase has finished!")
    }

    // requests to server are asynchronous, need to wait for server to respond
    private fun waitForRequestsToFinish() = runBlocking {
        println("Waiting for requests to finish")
        while (client.shortLivingRequestsCS.coroutineContext.job.children.toList().isNotEmpty()) {
            delay(500L)
        }
        println("Finished waiting!")
    }

    private fun buildProject(compiler: Compiler = Compiler.Gcc, buildDirName: String) {
        try {
            val buildCommand = getBuildCommand(compiler, buildDirName)
            ProcessBuilder("bash", "-c", buildCommand)
                .directory(testProjectPath.toFile())
                .inheritIO()
                .start()
                .waitFor()
            println("build command finished!")
            Files.list(testProjectPath).forEach {
                println(it)
            }
            Files.list(testProjectBuildDir).forEach {
                println(it)
            }
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }

    fun testGenerateForFile() {
        println("test testGenerateForFile has started!")
        myFixture.configureFromTempProjectFile("/lib/basic_functions.c")
        myFixture.performEditorAction("com.huawei.utbot.cpp.actions.GenerateForFileActionInEditor")
        waitForRequestsToFinish()
        checkFileExists(testProjectBuildDir, "build dir does not exist")
        checkFileExists(testProjectTestDir, "tests folder does not exist")
        checkFileExists(testProjectTestDir.resolve("lib/basic_functions_test.cpp"), "generated test file does not exist ")
        println("test testGenerateForFile has finished!")
    }

    // called after each test
    override fun tearDown() {
        println("tearDown of myUsefulTestCase is called")
        // somehow project service Client is not disposed automatically by the ide, and the exception is thrown that
        // timer related to heartbeat is not disposed. So let's dispose it manually.
        client.dispose()
        //testProjectBuildDir.delete(recursively = true)
        //testProjectTestDir.delete(recursively = true)
        super.tearDown()
        println("tearDown of myUseFulTestCase has finished!")
    }
}
