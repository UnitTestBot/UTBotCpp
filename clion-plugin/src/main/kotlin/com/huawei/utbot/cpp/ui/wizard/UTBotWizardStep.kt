package com.huawei.utbot.cpp.ui.wizard

import com.huawei.utbot.cpp.actions.utils.getDummyRequest
import com.huawei.utbot.cpp.client.GrpcClient
import com.huawei.utbot.cpp.services.UTBotSettings
import com.huawei.utbot.cpp.utils.commandLineEditor
import com.huawei.utbot.cpp.utils.validateOnInput
import com.intellij.ide.wizard.Step
import com.intellij.openapi.Disposable
import com.intellij.openapi.fileChooser.FileChooserDescriptorFactory
import com.intellij.openapi.project.Project
import com.intellij.openapi.ui.DialogPanel
import com.intellij.openapi.ui.ValidationInfo
import com.intellij.ui.components.JBLabel
import com.intellij.ui.components.JBTextField
import com.intellij.ui.dsl.builder.COLUMNS_LARGE
import com.intellij.ui.dsl.builder.COLUMNS_MEDIUM
import com.intellij.ui.dsl.builder.bindIntText
import com.intellij.ui.dsl.builder.bindText
import com.intellij.ui.dsl.builder.columns
import com.intellij.ui.dsl.builder.panel
import com.intellij.ui.layout.ComponentPredicate
import com.intellij.util.ui.HtmlPanel
import javax.swing.BoxLayout
import javax.swing.Icon
import javax.swing.JComponent
import javax.swing.JPanel
import kotlin.properties.Delegates
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.launch
import java.awt.Component
import java.awt.Dimension


abstract class UTBotWizardStep : Step {
    protected val panel by lazy { JPanel() }
    private var isInitialized = false
    private val onApplyCallbacks = mutableListOf<() -> Unit>()

    abstract fun createUI()

    override fun _init() {
        if (!isInitialized) {
            panel.layout = BoxLayout(panel, BoxLayout.Y_AXIS)
            panel.preferredSize = Dimension(800, 400)
            panel.minimumSize = panel.preferredSize
            createUI()
            isInitialized = true
        }
    }

    fun DialogPanel.addToUI() {
        alignmentX = Component.LEFT_ALIGNMENT
        panel.add(this)
        onApplyCallbacks.add { apply() }
    }

    override fun _commit(finishChosen: Boolean) {
        onApplyCallbacks.forEach {
            it.invoke()
        }
    }

    override fun getIcon(): Icon? {
        return null
    }

    override fun getComponent(): JComponent {
        return panel
    }

    override fun getPreferredFocusedComponent(): JComponent? {
        return panel
    }

    private fun getTextResource(resource: String): String {
        return this.javaClass.classLoader.getResource(resource)?.readText()
            ?: error("Unable to get resource: $resource")
    }

    fun addHtml(htmlResource: String) {
        panel.add(createHtmlComponent(getTextResource(htmlResource)))
    }

    private fun createHtmlComponent(html: String): JComponent {
        return object : HtmlPanel() {
            init {
                update()
                alignmentX = Component.LEFT_ALIGNMENT
                adjustHeightToTextHeight()
            }

            override fun getBody() = html

            fun adjustHeightToTextHeight() {
                // set dummy size, to update preferred
                size = Dimension(100, Short.MAX_VALUE.toInt())
                size = preferredSize
                minimumSize = preferredSize
                maximumSize = preferredSize
                update()
            }
        }
    }
}

class IntroStrep : UTBotWizardStep() {
    override fun createUI() {
        addHtml("media/intro.html")
    }
}

class ObservableValue<T>(initialValue: T) {
    private val changeListeners: MutableList<(T) -> Unit> = mutableListOf()
    var value: T by Delegates.observable(initialValue) { _, _, newVal ->
        changeListeners.forEach {
            it(newVal)
        }
    }

    fun addListener(listener: (T) -> Unit) {
        changeListeners.add(listener)
    }
}


class ConnectionStep(
    val project: Project,
    private val settingsModel: UTBotSettings.State,
    private val parentDisposable: Disposable
) : UTBotWizardStep() {
    private lateinit var hostTextField: JBTextField
    private lateinit var portTextField: JBTextField

    private val cs = CoroutineScope(Dispatchers.IO + SupervisorJob())

    private val pingedServer = ObservableValue<Boolean?>(null)
    private val isPingingServer = ObservableValue<Boolean>(false)
    private val isValidInput = ObservableValue<Boolean>(true)

    private suspend fun pingServer(port: Int, host: String) {
        GrpcClient(port, host, "DummyId").apply {
            use {
                stub.handshake(getDummyRequest())
            }
        }
    }

    private fun setupValidation() {
        portTextField.validateOnInput(parentDisposable) {
            if (portTextField.text.toIntOrNull() == null) {
                isValidInput.value = false
                ValidationInfo("Integer number expected!", portTextField)
            } else {
                isValidInput.value = true
                null
            }
        }
    }

    override fun createUI() {
        addHtml("media/connection.html")
        panel {
            row("Host") {
                textField().also {
                    it.bindText(settingsModel::serverName)
                    hostTextField = it.component
                }.columns(COLUMNS_MEDIUM)
            }
            row("Port") {
                intTextField().also {
                    it.bindIntText(settingsModel::port)
                }.columns(COLUMNS_MEDIUM).applyToComponent {
                    portTextField = this
                }
            }
            row {
                button("Test Connection") {
                    cs.launch {
                        isPingingServer.value = true
                        try {
                            pingServer(portTextField.text.toInt(), hostTextField.text)
                            pingedServer.value = true
                        } catch (e: io.grpc.StatusException) {
                            pingedServer.value = false
                        } finally {
                            isPingingServer.value = false
                        }
                    }
                }.enabledIf(object : ComponentPredicate() {
                    override fun invoke(): Boolean = !isPingingServer.value && isValidInput.value
                    override fun addListener(listener: (Boolean) -> Unit) {
                        isPingingServer.addListener { listener(!it) }
                        isValidInput.addListener(listener)
                    }
                })

                cell(JBLabel(com.intellij.ui.AnimatedIcon.Default())).visibleIf(object : ComponentPredicate() {
                    override fun invoke() = isPingingServer.value
                    override fun addListener(listener: (Boolean) -> Unit) {
                        isPingingServer.addListener(listener)
                    }
                })

                label("Successfully pinged the server!").visibleIf(object : ComponentPredicate() {
                    override fun invoke() = pingedServer.value == true
                    override fun addListener(listener: (Boolean) -> Unit) {
                        pingedServer.addListener { listener(it == true) }
                    }
                })

                label("Unable to ping the server!").visibleIf(object : ComponentPredicate() {
                    override fun invoke() = pingedServer.value == false
                    override fun addListener(listener: (Boolean) -> Unit) {
                        pingedServer.addListener { listener(it == false) }
                    }
                })
            }
        }.addToUI()

        setupValidation()
    }
}

class RemotePathStep(private val project: Project, private val settingsModel: UTBotSettings.State) :
    UTBotWizardStep() {
    override fun createUI() {
        addHtml("media/remote_path.html")
        panel {
            row {
                textField()
                    .bindText(settingsModel::remotePath)
                    .columns(COLUMNS_LARGE)
            }
        }.addToUI()
    }
}

class BuildOptionsStep(private val project: Project, private val settingsModel: UTBotSettings.State) :
    UTBotWizardStep() {
    override fun createUI() {
        addHtml("media/build_dir.html")
        panel {
            row("Relative Path to Build Folder") {
                textField()
                    .bindText(settingsModel::buildDirRelativePath)
                    .columns(COLUMNS_LARGE)
            }
        }.addToUI()
        addHtml("media/cmake_options.html")
        panel {
            row {
                commandLineEditor(
                    { settingsModel.cmakeOptions.joinToString(" ") },
                    { value: String -> settingsModel.cmakeOptions = value.split(" ") }
                )
            }
        }.addToUI()
    }
}

class SuccessStep : UTBotWizardStep() {
    override fun createUI() {
        addHtml("media/success.html")
    }
}
