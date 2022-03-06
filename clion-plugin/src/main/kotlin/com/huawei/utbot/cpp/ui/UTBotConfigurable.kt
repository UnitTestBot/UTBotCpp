package com.huawei.utbot.cpp.ui

import com.huawei.utbot.cpp.UTBot
import com.huawei.utbot.cpp.messaging.UTBotSettingsChangedListener
import com.huawei.utbot.cpp.services.GeneratorSettings
import com.huawei.utbot.cpp.services.UTBotSettings
import com.intellij.openapi.components.service
import com.intellij.openapi.diagnostic.Logger
import com.intellij.openapi.fileChooser.FileChooser
import com.intellij.openapi.fileChooser.FileChooserDescriptorFactory
import com.intellij.openapi.options.BoundConfigurable
import com.intellij.openapi.project.Project
import com.intellij.openapi.ui.DialogPanel
import com.intellij.openapi.ui.TextFieldWithBrowseButton
import com.intellij.ui.CollectionListModel
import com.intellij.ui.ToolbarDecorator
import com.intellij.ui.layout.panel
import javax.swing.JList
import java.awt.Dimension

/**
 * Get UTBot project settings and generation settings from settings menu.
 */
class UTBotConfigurable(private val targetProject: Project) : BoundConfigurable(
    "Project Settings for Generating Tests"
) {
    private val utbotSettings: UTBotSettings get() = targetProject.service()
    private val generatorSettings: GeneratorSettings get() = targetProject.service()
    private val logger = Logger.getInstance("ProjectConfigurable")
    private val sourcePathListModel =
        CollectionListModel(*targetProject.getService(UTBotSettings::class.java).sourcePaths.toTypedArray())
    private val onApplyCallBacks = mutableListOf<() -> Unit>()
    private val onResetCallBacks = mutableListOf<() -> Unit>()
    private val panel = createMainPanel()

    init {
        targetProject.messageBus.connect().subscribe(UTBotSettingsChangedListener.TOPIC, UTBotSettingsChangedListener {
            reset()
        })
    }

    override fun createPanel() = panel

    fun createMainPanel(): DialogPanel {
        logger.info("createPanel was called")
        fun TextFieldWithBrowseButton.setMaxSize() {
            maximumSize = TEXT_FIELD_MAX_SIZE
        }
        return panel {
            row {
                label(UTBot.message("settings.project.buildDir"))
                textFieldWithBrowseButton(
                    utbotSettings::buildDirPath,
                    UTBot.message("settings.project.buildDir.browse.title"),
                    targetProject, FileChooserDescriptorFactory.createSingleFileDescriptor()
                ).component.apply {
                    setMaxSize()
                    onApplyCallBacks.add { utbotSettings.buildDirPath = this.text }
                    onResetCallBacks.add { this.text = utbotSettings.buildDirPath }
                }
            }
            row {
                label(UTBot.message("settings.project.target"))
                textFieldWithBrowseButton(
                    utbotSettings::targetPath,
                    UTBot.message("settings.project.target.browse.title"),
                    project = targetProject,
                    fileChooserDescriptor = FileChooserDescriptorFactory.createSingleFileDescriptor()
                ).component.apply {
                    setMaxSize()
                    onApplyCallBacks.add { utbotSettings.targetPath = this.text }
                    onResetCallBacks.add { this.text = utbotSettings.targetPath }
                }
            }
            row {
                label(UTBot.message("settings.project.testsDir"))
                textFieldWithBrowseButton(
                    utbotSettings::testDirPath,
                    UTBot.message("settings.project.testsDir.browse.title"),
                    project = targetProject,
                    fileChooserDescriptor = FileChooserDescriptorFactory.createSingleFolderDescriptor()
                ).component.apply {
                    setMaxSize()
                    onApplyCallBacks.add { utbotSettings.testDirPath = this.text }
                    onResetCallBacks.add { this.text = utbotSettings.testDirPath }
                }
            }
            val checkBoxDs = mapOf(
                UTBot.message("settings.generation.synchronize") to utbotSettings::synchronizeCode,
                UTBot.message("settings.generation.stubs") to generatorSettings::useStubs,
                UTBot.message("settings.generation.verbose") to generatorSettings::verbose,
                UTBot.message("settings.generation.searcher") to generatorSettings::useDeterministicSearcher,
                UTBot.message("settings.generation.static") to generatorSettings::generateForStaticFunctions
            )
            checkBoxDs.forEach { message, boolProperty ->
                row {
                    val cb = checkBox(message, boolProperty)
                    cb.component.addItemListener {
                        boolProperty.set(!boolProperty.get())
                    }
                }
            }
            val intFields = mapOf(
                UTBot.message("settings.generation.timeoutFunction") to generatorSettings::timeoutPerFunction,
                UTBot.message("settings.generation.timeoutTest") to generatorSettings::timeoutPerTest,
                UTBot.message("settings.project.port") to utbotSettings::port
            )
            intFields.forEach { (message, intProperty) ->
                row(message) {
                    intTextField(intProperty).component.apply {
                        this.maximumSize = INT_FIELD_MAX_SIZE
                        onApplyCallBacks.add { intProperty.set(this.text.toInt()) }
                        onResetCallBacks.add { this.text = intProperty.get().toString() }
                    }
                }
            }
            row(UTBot.message("settings.project.serverName")) {
                textField(utbotSettings::serverName).component.apply {
                    this.maximumSize = TEXT_FIELD_MAX_SIZE
                    onResetCallBacks.add { this.text = utbotSettings.serverName }
                    onApplyCallBacks.add { utbotSettings.serverName = this.text }
                }
            }
            row(UTBot.message("settings.project.sourcePaths")) {
                component(createSourcesListComponent())
            }
            row(UTBot.message("settings.project.remotePath")) {
                textField(utbotSettings::remotePath).component.apply {
                    this.maximumSize = TEXT_FIELD_MAX_SIZE
                    onApplyCallBacks.add { utbotSettings.remotePath = this.text }
                    onResetCallBacks.add { this.text = utbotSettings.remotePath }
                }
            }
            row {
                label("Try to get paths from CMake model: ")
                button("detect paths") {
                    utbotSettings.predictPaths()
                    utbotSettings.fireUTBotSettingsChanged()
                }
            }
        }
    }

    private fun createSourcesListComponent() =
        ToolbarDecorator.createDecorator(JList(sourcePathListModel))
            .setAddAction { actionBtn ->
                FileChooser.chooseFiles(
                    FileChooserDescriptorFactory.createMultipleFoldersDescriptor(), targetProject, null
                ) { files ->
                    sourcePathListModel.add(files.map { it.path })
                }
            }.setRemoveAction { actionBtn ->
                sourcePathListModel.remove((actionBtn.contextComponent as JList<String>).selectedIndex)
            }.setPreferredSize(SOURCES_LIST_SIZE)
            .createPanel()

    override fun isModified(): Boolean {
        return super.isModified() || (sourcePathListModel.toList() != utbotSettings.sourcePaths)
    }

    override fun apply() {
        onApplyCallBacks.forEach { it() }
        utbotSettings.sourcePaths = sourcePathListModel.toList()
        utbotSettings.fireUTBotSettingsChanged()
    }

    override fun reset() {
        onResetCallBacks.forEach { it() }
        sourcePathListModel.also {
            it.removeAll()
            it.addAll(0, utbotSettings.sourcePaths)
        }
    }

    companion object {
        val TEXT_FIELD_MAX_SIZE = Dimension(370, 100)
        val INT_FIELD_MAX_SIZE = Dimension(100, 100)
        val SOURCES_LIST_SIZE = Dimension(500, 200)
    }
}
