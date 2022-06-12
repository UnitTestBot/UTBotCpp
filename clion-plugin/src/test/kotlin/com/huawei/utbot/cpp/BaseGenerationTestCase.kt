package com.huawei.utbot.cpp

import com.huawei.utbot.cpp.client.Client
import com.huawei.utbot.cpp.client.logger.SystemWriter
import com.huawei.utbot.cpp.services.GeneratorSettings
import com.huawei.utbot.cpp.services.UTBotSettings
import com.huawei.utbot.cpp.ui.targetsToolWindow.UTBotTargetsController
import com.huawei.utbot.cpp.utils.getClient
import com.huawei.utbot.cpp.utils.logger
import com.intellij.openapi.components.service
import com.intellij.openapi.diagnostic.trace
import com.intellij.openapi.project.Project
import com.intellij.testFramework.PlatformTestUtil
import com.intellij.testFramework.fixtures.CodeInsightTestFixture
import com.intellij.testFramework.fixtures.IdeaTestFixtureFactory
import com.intellij.testFramework.fixtures.impl.TempDirTestFixtureImpl
import com.intellij.util.io.delete
import kotlin.io.path.name
import kotlinx.coroutines.runBlocking
import org.junit.jupiter.api.AfterAll
import org.junit.jupiter.api.AfterEach
import org.junit.jupiter.api.TestInstance
import org.junit.jupiter.api.extension.ExtendWith
import java.io.File
import java.nio.file.Path
import java.nio.file.Paths

@TestInstance(TestInstance.Lifecycle.PER_CLASS)
@ExtendWith(SwingEdtInterceptor::class)
abstract class BaseGenerationTestCase {
    /**
     * Implementation of TempDirTestFixture that uses [testsDirectory] as
     * a tempDirectory, and does not delete it on tearDown.
     *
     * Intellij Platform tests are based on files in temp directory, which is provided and managed by TempDirTestFixture.
     * On tearDown, temp directory is deleted.
     * it may be expensive to copy all project files to temporary directory.
     * This class solves the problem, by using [testsDirectory]
     * instead of some generated temp directory.
     */
    class TestFixtureProxy(val testsDirectory: Path) : TempDirTestFixtureImpl() {
        override fun doCreateTempDirectory(): Path {
            return testsDirectory
        }

        // as the directory is not actually temporary, it should not be deleted
        override fun deleteOnTearDown() = false
    }

    init {
        Client.IS_TEST_MODE = true
    }

    val projectPath: Path =
        Paths.get(File(".").canonicalPath).resolve("../integration-tests/c-example-mini").normalize()
    val testsDirectoryPath: Path = projectPath.resolve("cl-plugin-test-tests")
    val buildDirName = "build"
    val buildDirectoryPath: Path
        get() = projectPath.resolve(buildDirName)
    val fixture: CodeInsightTestFixture = createFixture()
    val project: Project
        get() = fixture.project
    val settings: UTBotSettings
        get() = project.service()
    val generatorSettings: GeneratorSettings
        get() = project.service()
    val client: Client
        get() = project.getClient()
    val logger = com.intellij.openapi.diagnostic.Logger.getInstance(this.javaClass)
    val targetsController = UTBotTargetsController(project)

    init {
        settings.buildDirRelativePath = buildDirName
        settings.testDirPath = testsDirectoryPath.toString()
        project.logger.writers.let {
            it.clear()
            it.add(SystemWriter())
        }
    }

    private fun createFixture(): CodeInsightTestFixture {
        println("Creating fixture")
        logger.trace { "Creating fixture" }
        val fixture = IdeaTestFixtureFactory.getFixtureFactory().let {
            it.createCodeInsightFixture(
                it.createFixtureBuilder(projectPath.name, projectPath, false).fixture,
                TestFixtureProxy(projectPath)
            )
        }
        println("Before doing setUP")
        fixture.setUp()
        fixture.testDataPath = projectPath.toString()
        logger.trace { "Finished creating fixture" }
        println("Finished creating fixture")
        return fixture
    }

    fun setTarget(targetName: String) {
        assert(client.isServerAvailable()) { "Not connected to server!" }
        targetsController.requestTargetsFromServer()
        waitForRequestsToFinish()
        PlatformTestUtil.dispatchAllInvocationEventsInIdeEventQueue()
        targetsController.setTargetByName(targetName)
    }

    /**
     * Waits until all coroutines in client#shortLivingRequestsCS scope are finished.
     */
    fun waitForRequestsToFinish() = runBlocking {
        // requests to server are asynchronous, need to wait for server to respond
        logger.trace { "Waiting for requests to finish." }
        client.waitForServerRequestsToFinish()
        logger.trace { "Finished waiting!" }
    }

    @AfterEach
    fun tearDown() {
        logger.trace { "tearDown is called!" }
        buildDirectoryPath.delete(recursively = true)
        testsDirectoryPath.delete(recursively = true)
    }

    @AfterAll
    fun tearDownAll() {
        logger.trace { "tearDownAll of BaseGenerationTest is called" }
        waitForRequestsToFinish()
        fixture.tearDown()
        logger.trace { "tearDownAll of BaseGenerationTest has finished!" }
    }
}
