package com.huawei.utbot.cpp.services

import com.huawei.utbot.cpp.messaging.UTBotTestResultsReceivedListener
import com.huawei.utbot.cpp.models.TestNameAndTestSuite
import com.huawei.utbot.cpp.utils.utbotSettings
import com.intellij.codeInsight.daemon.DaemonCodeAnalyzer
import com.intellij.icons.AllIcons
import com.intellij.openapi.components.Service
import com.intellij.openapi.diagnostic.Logger
import com.intellij.openapi.fileEditor.FileEditorManager
import com.intellij.openapi.project.Project
import com.intellij.openapi.vfs.VirtualFileManager
import com.intellij.openapi.vfs.newvfs.BulkFileListener
import com.intellij.openapi.vfs.newvfs.events.VFileEvent
import com.intellij.psi.PsiElement
import javax.swing.Icon
import testsgen.Testgen
import java.nio.file.Paths
import java.util.concurrent.ConcurrentHashMap

@Service
class TestsResultsStorage(val project: Project) {
    private val storage: MutableMap<String, Testgen.TestResultObject> = ConcurrentHashMap(mutableMapOf())
    private val log = Logger.getInstance(this::class.java)

    init {
        val connection = project.messageBus.connect()
        connection.subscribe(UTBotTestResultsReceivedListener.TOPIC,
            UTBotTestResultsReceivedListener { results ->
                log.info("Received results")
                results.forEach { testResult ->
                    log.info("Result: ${testResult.testname} status: ${testResult.status}")
                    storage[testResult.testname] = testResult
                }

                forceGutterIconsUpdate()
            })

        connection.subscribe(VirtualFileManager.VFS_CHANGES, object : BulkFileListener {
            override fun after(events: MutableList<out VFileEvent>) {
                var wasSave = false
                events.forEach { event ->
                    if (event.isFromSave) {
                        wasSave = true
                        storage.forEach { entry ->
                            if (entry.value.testFilePath != event.path) {
                                storage.remove(entry.key)
                            }
                        }
                    }
                }

                if (wasSave) {
                    forceGutterIconsUpdate()
                }
            }
        })

    }

    private fun shouldForceUpdate(): Boolean {
        val currentlyOpenedFilePaths = FileEditorManager.getInstance(project).selectedEditors.mapNotNull {
            it.file?.toNioPath()
        }
        for (testResult in storage.values) {
            if (Paths.get(project.utbotSettings.convertFromRemotePathIfNeeded(testResult.testFilePath)) in currentlyOpenedFilePaths) {
                return true
            }
        }
        return false
    }

    private fun forceGutterIconsUpdate() {
        if (shouldForceUpdate())
            DaemonCodeAnalyzer.getInstance(project).restart()
    }

    fun getTestStatusIcon(element: PsiElement): Icon {
        log.info("getTestStatusIcon was called: $element")
        if (element.text == "UTBot") {
            return AllIcons.RunConfigurations.TestState.Run_run
        }

        val testName: String = TestNameAndTestSuite.getFromPsiElement(element).name

        if (!storage.contains(testName) || testName.isEmpty()) {
            return AllIcons.RunConfigurations.TestState.Run
        }

        return when (storage[testName]!!.status) {
            Testgen.TestStatus.TEST_FAILED -> AllIcons.RunConfigurations.TestState.Red2
            Testgen.TestStatus.TEST_PASSED -> AllIcons.RunConfigurations.TestState.Green2
            else -> AllIcons.RunConfigurations.TestError
        }
    }
}
