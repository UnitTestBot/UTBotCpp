package com.huawei.utbot.cpp.ui

import com.huawei.utbot.cpp.UTBot
import com.huawei.utbot.cpp.messaging.UTBotSettingsChangedListener
import com.huawei.utbot.cpp.services.GeneratorSettings
import com.huawei.utbot.cpp.services.UTBotSettings
import com.huawei.utbot.cpp.utils.commandLineEditor
import com.huawei.utbot.cpp.utils.removeIndices
import com.intellij.openapi.components.service
import com.intellij.openapi.diagnostic.Logger
import com.intellij.openapi.fileChooser.FileChooser
import com.intellij.openapi.fileChooser.FileChooserDescriptorFactory
import com.intellij.openapi.options.BoundConfigurable
import com.intellij.openapi.project.Project
import com.intellij.openapi.ui.DialogPanel
import com.intellij.ui.CollectionListModel
import com.intellij.ui.ToolbarDecorator
import com.intellij.ui.dsl.builder.BottomGap
import com.intellij.ui.dsl.builder.COLUMNS_LARGE
import com.intellij.ui.dsl.builder.Panel
import com.intellij.ui.dsl.builder.Row
import com.intellij.ui.dsl.builder.TopGap
import com.intellij.ui.dsl.builder.bindIntText
import com.intellij.ui.dsl.builder.bindSelected
import com.intellij.ui.dsl.builder.bindText
import com.intellij.ui.dsl.builder.columns
import com.intellij.ui.dsl.builder.panel
import javax.swing.JList
import kotlin.reflect.KMutableProperty0
import java.awt.Dimension

/**
 * Get UTBot project settings and generation settings from settings menu.
 */
class UTBotConfigurable(private val myProject: Project) : BoundConfigurable(
    "Project Settings for Generating Tests"
) {
    private val utbotSettings: UTBotSettings get() = myProject.service()
    private val generatorSettings: GeneratorSettings get() = myProject.service()
    private val logger = Logger.getInstance("ProjectConfigurable")
    private val sourcePathListModel =
        CollectionListModel(*myProject.getService(UTBotSettings::class.java).sourcePaths.toTypedArray())
    private val panel by lazy { createMainPanel() }

    init {
        myProject.messageBus.connect()
            .subscribe(UTBotSettingsChangedListener.TOPIC, UTBotSettingsChangedListener {
                reset()
            })
    }

    override fun createPanel() = panel

    fun Panel.createPathChooser(property: KMutableProperty0<String>, name: String, chooserTitle: String): Row {
        return row(name) {
            textFieldWithBrowseButton(
                chooserTitle,
                myProject,
                FileChooserDescriptorFactory.createSingleFileDescriptor()
            ).bindText(property).columns(COLUMNS_LARGE)
        }
    }

    fun createMainPanel(): DialogPanel {
        logger.trace("createPanel was called")
        return panel {
            group("Connection Settings") {
                row(UTBot.message("settings.project.port")) {
                    intTextField().bindIntText(utbotSettings::port).applyToComponent {
                        maximumSize = TEXT_FIELD_MAX_SIZE
                    }
                }.rowComment(UTBot.message("deployment.utbotPort.description"))
                row(UTBot.message("settings.project.serverName")) {
                    textField().bindText(utbotSettings::serverName)
                }.rowComment(UTBot.message("deployment.utbotHost.description"))
                row(UTBot.message("settings.project.remotePath")) {
                    textField().bindText(utbotSettings::remotePath)
                }.rowComment(UTBot.message("deployment.remotePath.description"))
            }

            group("Paths") {
                createPathChooser(
                    utbotSettings::buildDirRelativePath,
                    UTBot.message("settings.project.buildDir"),
                    UTBot.message("settings.project.buildDir.browse.title")
                ).rowComment(UTBot.message("paths.buildDirectory.description"))
                createPathChooser(
                    utbotSettings::targetPath,
                    UTBot.message("settings.project.target"),
                    UTBot.message("settings.project.target.browse.title")
                ).rowComment(UTBot.message("paths.target.description"))
                createPathChooser(
                    utbotSettings::testDirPath,
                    UTBot.message("settings.project.testsDir"),
                    UTBot.message("settings.project.testsDir.browse.title")
                ).rowComment(UTBot.message("paths.testsDirectory.description"))

                row(UTBot.message("settings.project.sourcePaths")) {
                    cell(createSourcesListComponent())
                        .onApply {
                            utbotSettings.sourcePaths = sourcePathListModel.toList().toMutableSet()
                        }
                        .onReset {
                            sourcePathListModel.also {
                                it.removeAll()
                                it.addAll(0, utbotSettings.sourcePaths.toList())
                            }
                        }
                        .onIsModified {
                            (sourcePathListModel.toList() != utbotSettings.sourcePaths)
                        }
                    topGap(TopGap.SMALL)
                    bottomGap(BottomGap.SMALL)
                }.rowComment(UTBot.message("paths.sourceDirectories.description"))

                row {
                    label("Try to get paths from CMake model: ")
                    button("Detect Paths") {
                        utbotSettings.predictPaths()
                        utbotSettings.fireUTBotSettingsChanged()
                    }
                }.rowComment("Queries CMake configurations in order to get source paths, build path. Also predicts tests folder")
            }

            group("CMake") {
                row(UTBot.message("settings.project.cmakeOptions")) {
                    commandLineEditor(
                        { utbotSettings.cmakeOptions },
                        { utbotSettings.cmakeOptions = it }
                    )
                }.rowComment(UTBot.message("paths.cmakeOptions.description"))
            }

            data class CheckBoxInfo(
                val boolProperty: KMutableProperty0<Boolean>,
                val title: String,
                val description: String
            ) {
                fun add(panel: Panel) {
                    panel.row {
                        checkBox(title).bindSelected(boolProperty)
                    }.rowComment(description)
                }
            }

            group("Generator settings") {
                val checkBoxes = listOf(
                    CheckBoxInfo(
                        generatorSettings::useStubs,
                        UTBot.message("stubs.useStubs.title"),
                        UTBot.message("stubs.useStubs.description")
                    ),
                    CheckBoxInfo(
                        generatorSettings::verbose,
                        UTBot.message("testsGeneration.verboseFormatting.title"),
                        UTBot.message("testsGeneration.verboseFormatting.description")
                    ),
                    CheckBoxInfo(
                        generatorSettings::useDeterministicSearcher,
                        UTBot.message("advanced.useDeterministicSearcher.title"),
                        UTBot.message("advanced.useDeterministicSearcher.description")
                    ),
                    CheckBoxInfo(
                        generatorSettings::generateForStaticFunctions,
                        UTBot.message("testsGeneration.generateForStaticFunctions.title"),
                        UTBot.message("testsGeneration.generateForStaticFunctions.description")
                    )
                )
                checkBoxes.forEach {
                    it.add(this)
                }

                row(UTBot.message("advanced.timeoutPerFunction.title")) {
                    intTextField().bindIntText(generatorSettings::timeoutPerFunction).applyToComponent {
                        maximumSize = TEXT_FIELD_MAX_SIZE
                    }
                }.rowComment(UTBot.message("advanced.timeoutPerFunction.description"))

                row(UTBot.message("advanced.timeoutPerTest.title")) {
                    intTextField().bindIntText(generatorSettings::timeoutPerFunction).applyToComponent {
                        maximumSize = TEXT_FIELD_MAX_SIZE
                    }
                }.rowComment(UTBot.message("advanced.timeoutPerTest.description"))
            }

        }
    }

    private fun createSourcesListComponent() =
        ToolbarDecorator.createDecorator(JList(sourcePathListModel))
            .setAddAction { _ ->
                FileChooser.chooseFiles(
                    FileChooserDescriptorFactory.createMultipleFoldersDescriptor(), myProject, null
                ) { files ->
                    sourcePathListModel.add(files.map { it.path })
                }
            }.setRemoveAction { actionBtn ->
                sourcePathListModel.removeIndices((actionBtn.contextComponent as JList<String>).selectedIndices)
            }.setPreferredSize(SOURCES_LIST_SIZE)
            .createPanel()

    override fun isModified(): Boolean {
        return panel.isModified()
    }

    override fun apply() {
        panel.apply()
        utbotSettings.fireUTBotSettingsChanged()
    }

    override fun reset() {
        panel.reset()
    }

    companion object {
        val TEXT_FIELD_MAX_SIZE = Dimension(370, 100)
        val SOURCES_LIST_SIZE = Dimension(500, 200)
    }
}
