@file:Suppress("UnstableApiUsage")

package org.utbot.cpp.clion.plugin.settings

import com.intellij.openapi.components.service
import com.intellij.openapi.diagnostic.Logger
import com.intellij.openapi.options.BoundConfigurable
import com.intellij.openapi.options.ConfigurationException
import com.intellij.openapi.project.Project
import com.intellij.openapi.ui.DialogPanel
import com.intellij.ui.JBIntSpinner
import com.intellij.ui.components.JBCheckBox
import com.intellij.ui.components.JBTextField
import com.intellij.ui.dsl.builder.BottomGap
import com.intellij.ui.dsl.builder.COLUMNS_LARGE
import com.intellij.ui.dsl.builder.Cell
import com.intellij.ui.dsl.builder.LabelPosition
import com.intellij.ui.dsl.builder.Panel
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
import org.utbot.cpp.clion.plugin.utils.ComponentValidationInfo
import org.utbot.cpp.clion.plugin.utils.ValidationCondition
import org.utbot.cpp.clion.plugin.utils.addValidation
import org.utbot.cpp.clion.plugin.utils.commandLineEditor
import org.utbot.cpp.clion.plugin.utils.isLookLikeUnixPath
import org.utbot.cpp.clion.plugin.utils.isValidHostName
import org.utbot.cpp.clion.plugin.utils.projectLifetimeDisposable
import java.awt.Dimension
import java.awt.event.ItemEvent

class UTBotConfigurable(private val myProject: Project) : BoundConfigurable(
    "Project Settings to Generate Tests"
) {
    private val logger = Logger.getInstance("ProjectConfigurable")
    private val panel by lazy { createMainPanel() }

    private val settings: UTBotProjectStoredSettings = myProject.service()
    private val validationInfos: MutableList<ComponentValidationInfo> = mutableListOf()
    private lateinit var portComponent: JBIntSpinner
    private lateinit var serverNameTextField: JBTextField
    private lateinit var pluginEnabledCheckBox: JBCheckBox
    private val isPluginEnabledObservable = ObservableValue<Boolean>(settings.isPluginEnabled)

    init {
        myProject.messageBus.connect(myProject.projectLifetimeDisposable)
            .subscribe(UTBotSettingsChangedListener.TOPIC, UTBotSettingsChangedListener {
                reset()
            })
    }

    private fun <T : JBTextField> Cell<T>.validateInput(vararg conditions: ValidationCondition): Cell<T> {
        return this.apply {
            validationInfos.add(this.addValidation(*conditions))
        }
    }


    override fun createPanel() = panel

    private fun createMainPanel(): DialogPanel {
        logger.trace("createPanel was called")
        return panel {
            createEnabledCheckBox()
            panel {
                group("Connection Settings") { createConnectionSettings() }
                group("Paths") { createPathsSettings() }
                group("CMake") { createCMakeSettings() }
                group("Generator Settings") { createGeneratorSettings() }
            }.visibleIf(object : ComponentPredicate() {
                override fun invoke(): Boolean = isPluginEnabledObservable.value

                override fun addListener(listener: (Boolean) -> Unit) {
                    isPluginEnabledObservable.addOnChangeListener(listener)
                }
            })
        }
    }

    private fun Panel.createEnabledCheckBox() {
        row(UTBot.message("settings.enabled.title")) {
            checkBox("").bindSelected(settings::isPluginEnabled).applyToComponent {
                pluginEnabledCheckBox = this
                addItemListener { itemEvent ->
                    if (itemEvent.stateChange == ItemEvent.SELECTED) {
                        isPluginEnabledObservable.value = true
                    } else if (itemEvent.stateChange == ItemEvent.DESELECTED) {
                        isPluginEnabledObservable.value = false
                    }
                }
            }
        }
    }

    private fun Panel.createConnectionSettings() {
        row(UTBot.message("settings.project.port")) {
            spinner(
                0..65535,
                1
            ).bindIntValue(projectIndependentSettings::port).applyToComponent {
                portComponent = this
            }
        }.rowComment(UTBot.message("deployment.utbot.port.description"))

        row(UTBot.message("settings.project.serverName")) {
            textField().bindText(projectIndependentSettings::serverName).applyToComponent {
                serverNameTextField = this
            }.validateInput(
                ValidationCondition(
                    UTBot.message("validation.invalid.host")
                ) { it.text.isValidHostName() }
            )
        }.rowComment(UTBot.message("deployment.utbot.host.description"))

        row(UTBot.message("settings.project.remotePath")) {
            textField().bindText(settings::remotePath).columns(COLUMNS_LARGE).validateInput(
                ValidationCondition(UTBot.message("validation.not.empty")) { it.text.isNotEmpty() },
                ValidationCondition(UTBot.message("validation.not.unix.path")) { it.text.isLookLikeUnixPath() }
            )
        }.rowComment(UTBot.message("deployment.remotePath.description"))
    }

    private fun Panel.createPathsSettings() {
        row(UTBot.message("settings.project.buildDir")) {
            val validator: (JBTextField) -> Boolean = {
                it.text.isNotEmpty()
            }
            textField().bindText(settings::buildDirRelativePath).columns(COLUMNS_LARGE)
                .validateInput(ValidationCondition(UTBot.message("validation.not.empty")) { it.text.isNotEmpty() })
        }.rowComment(UTBot.message("paths.buildDirectory.description"))

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
                .validateInput(ValidationCondition(UTBot.message("validation.not.empty")) { it.text.isNotEmpty() })
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
        val invalidComponentValidationInfo = validationInfos.find { !it.isValid() }
        if (invalidComponentValidationInfo != null) {
            panel.scrollRectToVisible(invalidComponentValidationInfo.component.visibleRect)
            invalidComponentValidationInfo.component.requestFocus()
            throw ConfigurationException("Some fields have invalid values")
        }
        val wereConnectionSettingsModified =
            portComponent.number != projectIndependentSettings.port || serverNameTextField.text != projectIndependentSettings.serverName
        val wasPluginEnabledChanged = pluginEnabledCheckBox.isSelected != settings.isPluginEnabled
        panel.apply()
        myProject.settings.fireUTBotSettingsChanged()
        if (wereConnectionSettingsModified)
            projectIndependentSettings.fireConnectionSettingsChanged()
        if (wasPluginEnabledChanged)
            myProject.service<UTBotAllProjectSettings>().fireUTBotEnabledStateChanged()
    }

    override fun reset() {
        panel.reset()
    }

    companion object {
        val TEXT_FIELD_MAX_SIZE = Dimension(370, 100)
    }
}
