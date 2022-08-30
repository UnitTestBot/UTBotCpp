package org.utbot.cpp.clion.plugin.ui.services

import com.intellij.codeInsight.daemon.DaemonCodeAnalyzer
import com.intellij.openapi.Disposable
import com.intellij.openapi.components.Service
import com.intellij.openapi.diagnostic.Logger
import com.intellij.openapi.fileEditor.FileEditorManager
import com.intellij.openapi.project.Project
import com.intellij.openapi.util.Disposer
import org.utbot.cpp.clion.plugin.client.handlers.SourceCode
import org.utbot.cpp.clion.plugin.listeners.UTBotTestResultsReceivedListener
import org.utbot.cpp.clion.plugin.utils.convertFromRemotePathIfNeeded
import org.utbot.cpp.clion.plugin.utils.projectLifetimeDisposable
import testsgen.Testgen
import java.util.concurrent.ConcurrentHashMap

@Service
class TestsResultsStorage(val project: Project) : Disposable {
    private val storage: MutableMap<String, Testgen.TestResultObject> = ConcurrentHashMap(mutableMapOf())
    private val log = Logger.getInstance(this::class.java)

    init {
        Disposer.register(project.projectLifetimeDisposable, this)
        project.messageBus.connect(this).subscribe(
            UTBotTestResultsReceivedListener.TOPIC,
            UTBotTestResultsReceivedListener { results ->
                log.info("Received results")
                results.forEach { testResult ->
                    log.info("Result: ${testResult.testname} status: ${testResult.status}")
                    storage[testResult.testname] = testResult
                }

                forceGutterIconsUpdate()
            })
    }

    fun getTestResultByTestName(testName: String): Testgen.TestResultObject? = storage[testName]

    /**
     * Cleans the results of previous test run if tests were regenerated.
     */
    fun clearTestResults(sourceCodes: List<SourceCode>) {
        val localFilePaths = sourceCodes.map { it.localPath }.toSet()
        storage.values.removeIf { it.testFilePath.convertFromRemotePathIfNeeded(project) in localFilePaths }
    }

    private fun shouldForceUpdate(): Boolean {
        val currentlyOpenedFilePaths = FileEditorManager.getInstance(project)
            .selectedEditors
            .mapNotNull { it.file?.toNioPath() }

        for (testResult in storage.values) {
            if (testResult.testFilePath.convertFromRemotePathIfNeeded(project) in currentlyOpenedFilePaths) {
                return true
            }
        }
        return false
    }

    private fun forceGutterIconsUpdate() {
        if (shouldForceUpdate())
            DaemonCodeAnalyzer.getInstance(project).restart()
    }

    override fun dispose() {
        storage.clear()
    }
}
