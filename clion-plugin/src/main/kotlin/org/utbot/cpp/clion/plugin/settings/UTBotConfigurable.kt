package org.utbot.cpp.clion.plugin.settings

import com.intellij.openapi.diagnostic.Logger
import com.intellij.openapi.fileChooser.FileChooserDescriptorFactory
import com.intellij.openapi.options.BoundConfigurable
import com.intellij.openapi.project.Project
import com.intellij.openapi.ui.DialogPanel
import com.intellij.ui.dsl.builder.BottomGap
import com.intellij.ui.dsl.builder.COLUMNS_LARGE
import com.intellij.ui.dsl.builder.Panel
import com.intellij.ui.dsl.builder.Row
import com.intellij.ui.dsl.builder.bindIntText
import com.intellij.ui.dsl.builder.bindSelected
import com.intellij.ui.dsl.builder.bindText
import com.intellij.ui.dsl.builder.columns
import com.intellij.ui.dsl.builder.panel
import kotlin.reflect.KMutableProperty0
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.listeners.UTBotSettingsChangedListener
import org.utbot.cpp.clion.plugin.ui.sourceFoldersView.UTBotProjectViewPaneForSettings
import org.utbot.cpp.clion.plugin.utils.commandLineEditor
import java.awt.Dimension

class UTBotConfigurable(private val myProject: Project) : BoundConfigurable(
    "Project Settings for Generating Tests"
) {
    private val logger = Logger.getInstance("ProjectConfigurable")
    private val panel by lazy { createMainPanel() }

    private val settings = myProject.settings.storedSettings

    init {
        myProject.messageBus.connect()
            .subscribe(UTBotSettingsChangedListener.TOPIC, UTBotSettingsChangedListener {
                reset()
            })
    }

    override fun createPanel() = panel

    private fun Panel.createPathChooser(property: KMutableProperty0<String>, name: String, chooserTitle: String): Row {
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
                    intTextField().bindIntText(projectIndependentSettings::port).applyToComponent {
                        maximumSize = TEXT_FIELD_MAX_SIZE
                    }
                }.rowComment(UTBot.message("deployment.utbotPort.description"))
                row(UTBot.message("settings.project.serverName")) {
                    textField().bindText(projectIndependentSettings::serverName)
                }.rowComment(UTBot.message("deployment.utbotHost.description"))
                row(UTBot.message("settings.project.remotePath")) {
                    textField().bindText(settings::remotePath).columns(COLUMNS_LARGE)
                }.rowComment(UTBot.message("deployment.remotePath.description"))
            }

            group("Paths") {
                row(UTBot.message("settings.project.projectPath")) {
                    textFieldWithBrowseButton(
                        UTBot.message("settings.project.projectPath.title"),
                        myProject,
                        FileChooserDescriptorFactory.createSingleFileDescriptor()
                    ).bindText(
                        getter = { myProject.settings.projectPath ?: "" },
                        setter = { value -> myProject.settings.projectPath = value })
                        .columns(COLUMNS_LARGE)
                }.rowComment(UTBot.message("settings.project.projectPath.info"))
                createPathChooser(
                    settings::buildDirRelativePath,
                    UTBot.message("settings.project.buildDir"),
                    UTBot.message("settings.project.buildDir.browse.title")
                ).rowComment(UTBot.message("paths.buildDirectory.description"))
                createPathChooser(
                    settings::targetPath,
                    UTBot.message("settings.project.target"),
                    UTBot.message("settings.project.target.browse.title")
                ).rowComment(UTBot.message("paths.target.description"))
                createPathChooser(
                    settings::testDirPath,
                    UTBot.message("settings.project.testsDir"),
                    UTBot.message("settings.project.testsDir.browse.title")
                ).rowComment(UTBot.message("paths.testsDirectory.description"))

                row(UTBot.message("settings.project.sourcePaths")) {
                    val pane = UTBotProjectViewPaneForSettings(myProject)
                    cell(pane.createComponent()).onApply {
                        pane.apply()
                    }.onReset {
                        pane.reset()
                    }.onIsModified {
                        pane.isModified()
                    }
                }.bottomGap(BottomGap.SMALL).rowComment(UTBot.message("paths.sourceDirectories.description"))

                row {
                    label("Try to get paths from CMake model: ")
                    button("Detect Paths") {
                        myProject.settings.predictPaths()
                        myProject.settings.fireUTBotSettingsChanged()
                    }
                }.rowComment("Queries CMake configurations in order to get source paths, build path. Also predicts tests folder")
            }

            group("CMake") {
                row(UTBot.message("settings.project.cmakeOptions")) {
                    commandLineEditor(
                        { settings.cmakeOptions },
                        { settings.cmakeOptions = it }
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
                        settings::useStubs,
                        UTBot.message("stubs.useStubs.title"),
                        UTBot.message("stubs.useStubs.description")
                    ),
                    CheckBoxInfo(
                        settings::verbose,
                        UTBot.message("testsGeneration.verboseFormatting.title"),
                        UTBot.message("testsGeneration.verboseFormatting.description")
                    ),
                    CheckBoxInfo(
                        settings::useDeterministicSearcher,
                        UTBot.message("advanced.useDeterministicSearcher.title"),
                        UTBot.message("advanced.useDeterministicSearcher.description")
                    ),
                    CheckBoxInfo(
                        settings::generateForStaticFunctions,
                        UTBot.message("testsGeneration.generateForStaticFunctions.title"),
                        UTBot.message("testsGeneration.generateForStaticFunctions.description")
                    )
                )
                checkBoxes.forEach {
                    it.add(this)
                }

                row(UTBot.message("advanced.timeoutPerFunction.title")) {
                    intTextField().bindIntText(settings::timeoutPerFunction).applyToComponent {
                        maximumSize = TEXT_FIELD_MAX_SIZE
                    }
                }.rowComment(UTBot.message("advanced.timeoutPerFunction.description"))

                row(UTBot.message("advanced.timeoutPerTest.title")) {
                    intTextField().bindIntText(settings::timeoutPerFunction).applyToComponent {
                        maximumSize = TEXT_FIELD_MAX_SIZE
                    }
                }.rowComment(UTBot.message("advanced.timeoutPerTest.description"))
            }

        }
    }

    override fun isModified(): Boolean {
        return panel.isModified()
    }

    override fun apply() {
        panel.apply()
        myProject.settings.fireUTBotSettingsChanged()
    }

    override fun reset() {
        panel.reset()
    }

    companion object {
        val TEXT_FIELD_MAX_SIZE = Dimension(370, 100)
    }
}