package org.utbot.cpp.clion.plugin.tests.integrationTests

import com.intellij.openapi.components.service
import com.intellij.openapi.project.Project
import com.intellij.testFramework.PlatformTestUtil
import com.intellij.testFramework.fixtures.CodeInsightTestFixture
import com.intellij.util.io.delete
import kotlinx.coroutines.Job
import kotlinx.coroutines.TimeoutCancellationException
import kotlinx.coroutines.delay
import kotlinx.coroutines.runBlocking
import kotlinx.coroutines.withTimeout
import org.junit.jupiter.api.AfterAll
import org.junit.jupiter.api.AfterEach
import org.junit.jupiter.api.TestInstance
import org.junit.jupiter.api.extension.ExtendWith
import org.tinylog.kotlin.Logger
import org.utbot.cpp.clion.plugin.SwingEdtInterceptor
import org.utbot.cpp.clion.plugin.client.ManagedClient
import org.utbot.cpp.clion.plugin.client.logger.SystemWriter
import org.utbot.cpp.clion.plugin.createFixture
import org.utbot.cpp.clion.plugin.settings.settings
import org.utbot.cpp.clion.plugin.ui.utbotToolWindow.targetToolWindow.UTBotTargetsController
import org.utbot.cpp.clion.plugin.utils.client
import org.utbot.cpp.clion.plugin.utils.logger
import java.io.File
import java.nio.file.Path
import java.nio.file.Paths

@TestInstance(TestInstance.Lifecycle.PER_CLASS)
@ExtendWith(SwingEdtInterceptor::class)
abstract class BaseGenerationTestCase {
    val projectPath: Path =
        Paths.get(File(".").canonicalPath).resolve("../integration-tests/c-example-mini").normalize()
    val testsDirectoryPath: Path = projectPath.resolve("cl-plugin-test-tests")
    val buildDirName = "build"
    protected val fixture: CodeInsightTestFixture = createFixture(projectPath)
    protected val project: Project
        get() = fixture.project
    protected val client: ManagedClient
        get() = project.client

    init {
        project.settings.storedSettings.buildDirRelPath = buildDirName
        project.settings.storedSettings.testDirRelativePath = projectPath.relativize(testsDirectoryPath).toString()
        project.settings.storedSettings.isPluginEnabled = true
        project.logger.logWriters.let {
            it.clear()
            it.add(SystemWriter())
        }
    }

    fun setTarget(targetName: String) {
        val targetsController = project.service<UTBotTargetsController>()
        assert(client.isServerAvailable()) { "Not connected to server!" }
        targetsController.requestTargetsFromServer()
        waitForRequestsToFinish()
        PlatformTestUtil.dispatchAllInvocationEventsInIdeEventQueue()
        targetsController.setTargetPathByName(targetName)
    }

    /**
     * Waits until all requests initiated during tests are finished
     */
    fun waitForRequestsToFinish() {
        // requests to server are asynchronous, need to wait for server to respond
        client.waitForServerRequestsToFinish(ifNotFinished = { unfinishedCoroutines: List<Job> ->
            // some requests may be executed only on EDT, so we flush the queue. Otherwise,
            // these requests won't finish
            PlatformTestUtil.dispatchAllInvocationEventsInIdeEventQueue()
            Logger.info("Waiting for requests to finish: $unfinishedCoroutines")
        })
        Logger.info("Finished waiting!")
    }

    protected fun waitForConnection(timeout: Long = 10000L) {
        Logger.info { "Waiting for connection to server!" }
        runBlocking {
            try {
                withTimeout(timeout) {
                    while (!client.isServerAvailable()) {
                        delay(1000L)
                    }
                }
            } catch (_: TimeoutCancellationException) {
            }
            assert(client.isServerAvailable()) { "Not connected to server!" }
            Logger.info { "Connected" }
        }
    }

    @AfterEach
    fun tearDown() {
        Logger.info("tearDown is called!")
        project.settings.buildDirPath.delete(recursively = true)
        testsDirectoryPath.delete(recursively = true)
    }

    @AfterAll
    fun tearDownAll() {
        Logger.info("tearDownAll of BaseGenerationTest is called")
        fixture.tearDown()
        Logger.info("tearDownAll of BaseGenerationTest has finished!")
    }
}
