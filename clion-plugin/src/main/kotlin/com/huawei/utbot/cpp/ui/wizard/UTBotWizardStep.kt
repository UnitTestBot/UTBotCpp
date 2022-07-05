package com.huawei.utbot.cpp.ui.wizard

import com.huawei.utbot.cpp.actions.utils.getDummyRequest
import com.huawei.utbot.cpp.actions.utils.getVersionInfo
import com.huawei.utbot.cpp.client.GrpcClient
import com.huawei.utbot.cpp.services.UTBotSettings
import com.huawei.utbot.cpp.utils.commandLineEditor
import com.huawei.utbot.cpp.utils.utbotSettings
import com.huawei.utbot.cpp.utils.validateOnInput
import com.intellij.ide.wizard.Step
import com.intellij.openapi.Disposable
import com.intellij.openapi.fileChooser.FileChooserDescriptorFactory
import com.intellij.openapi.project.Project
import com.intellij.openapi.ui.DialogPanel
import com.intellij.openapi.ui.ValidationInfo
import com.intellij.ui.DocumentAdapter
import com.intellij.ui.components.JBLabel
import com.intellij.ui.components.JBTextField
import com.intellij.ui.dsl.builder.COLUMNS_LARGE
import com.intellij.ui.dsl.builder.COLUMNS_MEDIUM
import com.intellij.ui.dsl.builder.bindIntText
import com.intellij.ui.dsl.builder.bindSelected
import com.intellij.ui.dsl.builder.bindText
import com.intellij.ui.dsl.builder.columns
import com.intellij.ui.dsl.builder.panel
import com.intellij.ui.layout.ComponentPredicate
import com.intellij.util.ui.HtmlPanel
import javax.swing.BoxLayout
import javax.swing.Icon
import javax.swing.JComponent
import javax.swing.JPanel
import javax.swing.event.DocumentEvent
import kotlin.properties.Delegates
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.delay
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

    enum class ConnectionStatus {
        connected, connecting, failed, warning
    }

    private val connectionStatus = ObservableValue<ConnectionStatus>(ConnectionStatus.failed)

    private suspend fun pingServer(port: Int, host: String): ConnectionStatus {
        connectionStatus.value = ConnectionStatus.connecting
        val client = GrpcClient(port, host, "DummyId")
        try {
            val serverVersion = client.stub.handshake(getVersionInfo()).version
            if (serverVersion != UTBotSettings.versionInfo)
                return ConnectionStatus.warning
            return ConnectionStatus.connected
        } catch (e: Throwable) {
            return ConnectionStatus.failed
        } finally {
            client.close()
        }
    }

    private fun pingServer() {
        cs.launch {
            connectionStatus.value = pingServer(portTextField.text.toInt(), hostTextField.text)
        }
    }

    override fun _init() {
        super._init()
        pingServer()
    }

    private fun setupValidation() {
        portTextField.validateOnInput(parentDisposable) {
            val value = portTextField.text.toUShortOrNull()
            if (value == null) {
                ValidationInfo("Number from 0 to 65535 is expected!", portTextField)
            } else {
                pingServer()
                null
            }
        }

        hostTextField.document.addDocumentListener(object : DocumentAdapter() {
            override fun textChanged(e: DocumentEvent) {
                pingServer()
            }
        })
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
                cell(JBLabel(com.intellij.ui.AnimatedIcon.Default())).visibleIf(object : ComponentPredicate() {
                    override fun invoke() = connectionStatus.value == ConnectionStatus.connecting
                    override fun addListener(listener: (Boolean) -> Unit) {
                        connectionStatus.addListener { listener(it == ConnectionStatus.connecting) }
                    }
                })

                label("✔️ Successfully pinged server!").visibleIf(object : ComponentPredicate() {
                    override fun invoke() = connectionStatus.value == ConnectionStatus.connected
                    override fun addListener(listener: (Boolean) -> Unit) {
                        connectionStatus.addListener { listener(it == ConnectionStatus.connected) }
                    }
                })

                label("❌ Failed to establish connection!").visibleIf(object : ComponentPredicate() {
                    override fun invoke() = connectionStatus.value == ConnectionStatus.failed
                    override fun addListener(listener: (Boolean) -> Unit) {
                        connectionStatus.addListener { listener(it == ConnectionStatus.failed) }
                    }
                })

                label("⚠️ Warning! Versions are different").visibleIf(object : ComponentPredicate() {
                    override fun invoke() = connectionStatus.value == ConnectionStatus.warning
                    override fun addListener(listener: (Boolean) -> Unit) {
                        connectionStatus.addListener { listener(it == ConnectionStatus.warning) }
                    }
                })
            }
            setupValidation()
        }.addToUI()

        addHtml("media/remote_path.html")
        panel {
            row {
                textField()
                    .bindText(settingsModel::remotePath)
                    .columns(COLUMNS_LARGE)
            }
            row {
                checkBox("Use defaults")
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
