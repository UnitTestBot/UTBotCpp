@file:Suppress("UnstableApiUsage")

package org.utbot.cpp.clion.plugin.settings

import com.intellij.openapi.components.service
import com.intellij.openapi.diagnostic.Logger
import com.intellij.openapi.fileChooser.FileChooserDescriptorFactory
import com.intellij.openapi.options.BoundConfigurable
import com.intellij.openapi.project.Project
import com.intellij.openapi.ui.DialogPanel
import com.intellij.ui.components.JBTextField
import com.intellij.ui.dsl.builder.BottomGap
import com.intellij.ui.dsl.builder.COLUMNS_LARGE
import com.intellij.ui.dsl.builder.LabelPosition
import com.intellij.ui.dsl.builder.Panel
import com.intellij.ui.dsl.builder.Row
import com.intellij.ui.dsl.builder.bindIntText
import com.intellij.ui.dsl.builder.bindIntValue
import com.intellij.ui.dsl.builder.bindSelected
import com.intellij.ui.dsl.builder.bindText
import com.intellij.ui.dsl.builder.columns
import com.intellij.ui.dsl.builder.panel
import com.intellij.ui.layout.ComponentPredicate
import kotlin.reflect.KMutableProperty0
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.listeners.UTBotSettingsChangedListener
import org.utbot.cpp.clion.plugin.ui.ObservableValue
import org.utbot.cpp.clion.plugin.ui.sourceFoldersView.UTBotProjectViewPaneForSettings
import org.utbot.cpp.clion.plugin.utils.commandLineEditor
import org.utbot.cpp.clion.plugin.utils.isWindows
import org.utbot.cpp.clion.plugin.utils.path
import org.utbot.cpp.clion.plugin.utils.toWslFormatIfNeeded
import java.awt.Dimension

class UTBotConfigurable(private val myProject: Project) : BoundConfigurable(
    "Project Settings to Generate Tests"
) {
    private val logger = Logger.getInstance("ProjectConfigurable")
    private val panel by lazy { createMainPanel() }

    private val settings: UTBotProjectStoredSettings = myProject.service()
    private lateinit var portTextfield: JBTextField
    private lateinit var serverNameTextField: JBTextField

    private val isLocalOrWsl = ObservableValue(settings.isLocalOrWslScenario)

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

    private fun createMainPanel(): DialogPanel {
        logger.trace("createPanel was called")
        return panel {
            group("Connection Settings") { createConnectionSettings() }
            group("Paths") { createPathsSettings() }
            group("CMake") { createCMakeSettings() }
            group("Generator Settings") { createGeneratorSettings() }
        }
    }

    private fun Panel.createConnectionSettings() {
        row(UTBot.message("settings.project.port")) {
            intTextField().bindIntText(projectIndependentSettings::port).applyToComponent {
                portTextfield = this
                maximumSize = TEXT_FIELD_MAX_SIZE
            }
        }.rowComment(UTBot.message("deployment.utbotPort.description"))

        row {
            checkBox("Local or WSL scenario")
                .bindSelected(settings::isLocalOrWslScenario)
                .applyToComponent {
                    this.addActionListener {
                        isLocalOrWsl.value = !isLocalOrWsl.value
                    }
                }
        }

        val enabledIfNotLocalOrWslScenario = object : ComponentPredicate() {
            override fun invoke(): Boolean = !isLocalOrWsl.value
            override fun addListener(listener: (Boolean) -> Unit) =
                isLocalOrWsl.addOnChangeListener { value -> listener(!value) }
        }

        row(UTBot.message("settings.project.serverName")) {
            textField().bindText(projectIndependentSettings::serverName).applyToComponent {
                serverNameTextField = this
                isLocalOrWsl.addOnChangeListener { newValue ->
                    if (newValue)
                        this.text = "localhost"
                }
            }.enabledIf(enabledIfNotLocalOrWslScenario)
        }.rowComment(UTBot.message("deployment.utbotHost.description"))

        row(UTBot.message("settings.project.remotePath")) {
            textField().bindText(settings::remotePath).columns(COLUMNS_LARGE)
                .applyToComponent {
                    isLocalOrWsl.addOnChangeListener { newValue ->
                        if (newValue)
                            this.text = if (isWindows) myProject.path.toWslFormatIfNeeded() else ""
                    }
                }.enabledIf(enabledIfNotLocalOrWslScenario)
        }.rowComment(UTBot.message("deployment.remotePath.description"))
    }

    private fun Panel.createPathsSettings() {
        createPathChooser(
            settings::buildDirRelativePath,
            UTBot.message("settings.project.buildDir"),
            UTBot.message("settings.project.buildDir.browse.title")
        ).rowComment(UTBot.message("paths.buildDirectory.description"))

        row(UTBot.message("settings.project.target")) {
            textField().bindText(
                getter = {
                    settings.uiTargetPath
                },
                setter = {}
            ).columns(COLUMNS_LARGE).enabled(false)
        }.rowComment(UTBot.message("paths.target.description"))

        row(UTBot.message("settings.project.testsDir")) {
            textField().bindText(settings::testDirRelativePath).columns(COLUMNS_LARGE)
        }.rowComment(UTBot.message("paths.testsDir.description"))

        row {
            val pane = UTBotProjectViewPaneForSettings(myProject)
            cell(pane.createComponent()).onApply {
                pane.apply()
            }.onReset {
                pane.reset()
            }.onIsModified {
                pane.isModified()
            }.label(UTBot.message("settings.project.sourcePaths"), LabelPosition.TOP)
        }.bottomGap(BottomGap.SMALL).rowComment(UTBot.message("paths.sourceDirectories.description"))
    }

    private fun Panel.createCMakeSettings() {
        row(UTBot.message("settings.project.cmakeOptions")) {
            commandLineEditor(
                { settings.cmakeOptions },
                { settings.cmakeOptions = it }
            )
        }.rowComment(UTBot.message("paths.cmakeOptions.description"))
    }

    private fun Panel.createGeneratorSettings() {
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
            spinner(
                UTBotProjectStoredSettings.TIMEOUT_PER_FUNCTION_MIN_VALUE..
                        UTBotProjectStoredSettings.TIMEOUT_PER_FUNCTION_MAX_VALUE
            ).bindIntValue(settings::timeoutPerFunction).applyToComponent {
                maximumSize = TEXT_FIELD_MAX_SIZE
            }
        }.rowComment(UTBot.message("advanced.timeoutPerFunction.description"))

        row(UTBot.message("advanced.timeoutPerTest.title")) {
            spinner(
                UTBotProjectStoredSettings.TIMEOUT_PER_TEST_MIN_VALUE..
                        UTBotProjectStoredSettings.TIMEOUT_PER_TEST_MAX_VALUE
            ).bindIntValue(settings::timeoutPerTest).applyToComponent {
                maximumSize = TEXT_FIELD_MAX_SIZE
            }
        }.rowComment(UTBot.message("advanced.timeoutPerTest.description"))
    }

    override fun isModified(): Boolean {
        return panel.isModified()
    }

    override fun apply() {
        val wereConnectionSettingsModified =
            portTextfield.text != projectIndependentSettings.port.toString() || serverNameTextField.text != projectIndependentSettings.serverName
        panel.apply()
        myProject.settings.fireUTBotSettingsChanged()
        if (wereConnectionSettingsModified)
            projectIndependentSettings.fireConnectionSettingsChanged()
    }

    override fun reset() {
        panel.reset()
    }

    companion object {
        val TEXT_FIELD_MAX_SIZE = Dimension(370, 100)
    }
}