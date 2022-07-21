package org.utbot.cpp.clion.plugin.ui.wizard.steps

import com.intellij.openapi.Disposable
import com.intellij.openapi.project.Project
import com.intellij.openapi.ui.DialogWrapper
import com.intellij.openapi.ui.ValidationInfo
import com.intellij.ui.AnimatedIcon
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
import com.intellij.ui.dsl.builder.selected
import com.intellij.ui.layout.ComponentPredicate
import javax.swing.JComponent
import javax.swing.event.DocumentEvent
import kotlin.properties.Delegates
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.launch
import org.utbot.cpp.clion.plugin.grpc.getVersionRequests
import org.utbot.cpp.clion.plugin.client.GrpcClient
import org.utbot.cpp.clion.plugin.settings.UTBotAllSettings
import org.utbot.cpp.clion.plugin.settings.UTBotSettingsModel
import org.utbot.cpp.clion.plugin.ui.wizard.UTBotWizardStep
import org.utbot.cpp.clion.plugin.utils.isWindows
import org.utbot.cpp.clion.plugin.utils.toWSLPathOnWindows
import org.utbot.cpp.clion.plugin.utils.utbotSettings
import org.utbot.cpp.clion.plugin.utils.validateOnInput

class ObservableValue<T>(initialValue: T) {
    private val changeListeners: MutableList<(T) -> Unit> = mutableListOf()
    var value: T by Delegates.observable(initialValue) { _, _, newVal ->
        changeListeners.forEach {
            it(newVal)
        }
    }

    fun addOnChangeListener(listener: (T) -> Unit) {
        changeListeners.add(listener)
    }
}

class NotConnectedWarningDialog(project: Project) : DialogWrapper(project) {
    init {
        title = "❌ Server is unreachable!"
        super.init()
    }

    override fun createCenterPanel(): JComponent {
        return panel {
            row {
                text(
                    """UTBot failed to establish connection with specified server. 
                       If you wish to continue anyway, press "Ok" button.
                    """.trimMargin(), TEXT_LENGTH
                )
            }
            row {
                text(
                    """In any case, you will need to specify correct port and host of UTBot server to use the plugin.
                       You can do it via CLion Settings -> Tools -> UTBot Settings
                    """.trimIndent(), TEXT_LENGTH
                )
            }
        }
    }

    companion object {
        const val TEXT_LENGTH = 100
    }
}


class ConnectionStep(
    val project: Project,
    private val settingsModel: UTBotSettingsModel,
    private val parentDisposable: Disposable
) : UTBotWizardStep() {
    private lateinit var hostTextField: JBTextField
    private lateinit var portTextField: JBTextField
    private lateinit var remotePathTextField: JBTextField
    private var serverVersion: String? = null

    private val cs = CoroutineScope(Dispatchers.IO + SupervisorJob())

    enum class ConnectionStatus {
        connected, connecting, failed, warning
    }

    private val connectionStatus = ObservableValue<ConnectionStatus>(ConnectionStatus.failed)
    private val useDefaults = ObservableValue<Boolean>(false)

    init {
        useDefaults.addOnChangeListener { newValue ->
            if (newValue) {
                portTextField.text = UTBotAllSettings.DEFAULT_PORT.toString()
                hostTextField.text = UTBotAllSettings.DEFAULT_HOST
                remotePathTextField.text = project.utbotSettings.projectPath
                if (isWindows)
                    remotePathTextField.text = toWSLPathOnWindows(remotePathTextField.text)
            }
        }
    }

    private suspend fun pingServer(port: Int, host: String): ConnectionStatus {
        connectionStatus.value = ConnectionStatus.connecting
        runCatching {
            GrpcClient(port, host, "DummyId").use { client ->
                serverVersion = client.stub.handshake(getVersionRequests()).version
                if (serverVersion != UTBotAllSettings.clientVersion)
                    return ConnectionStatus.warning
                return ConnectionStatus.connected
            }
        }.getOrElse { exception ->
            when (exception) {
                is io.grpc.StatusException -> return ConnectionStatus.failed
                else -> {
                    connectionStatus.value = ConnectionStatus.failed
                    throw exception
                }
            }
        }
    }

    override fun canProceedToNextStep(): Boolean {
        if (connectionStatus.value == ConnectionStatus.failed) {
            return NotConnectedWarningDialog(project).showAndGet()
        }
        return true
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
                connectionStatus.value = ConnectionStatus.failed
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

            row {
                checkBox("Default server configuration on localhost (or WSL2)")
                    .bindSelected(getter = { useDefaults.value }, setter = { newValue ->
                        useDefaults.value = newValue
                    }).selected.addListener { newValue ->
                        useDefaults.value = newValue
                    }
            }

            row("Host") {
                textField().also {
                    it.bindText(settingsModel::serverName)
                    hostTextField = it.component
                }.columns(COLUMNS_MEDIUM).enabledIf(object : ComponentPredicate() {
                    override fun invoke() = !useDefaults.value
                    override fun addListener(listener: (Boolean) -> Unit) {
                        useDefaults.addOnChangeListener { newValue -> listener(!newValue) }
                    }
                })
            }

            row("Port") {
                intTextField(
                    0..65535,
                    1
                ).also {
                    it.bindIntText(settingsModel::port)
                }.columns(COLUMNS_MEDIUM).applyToComponent {
                    portTextField = this
                }.enabledIf(object : ComponentPredicate() {
                    override fun invoke() = !useDefaults.value
                    override fun addListener(listener: (Boolean) -> Unit) {
                        useDefaults.addOnChangeListener { newValue -> listener(!newValue) }
                    }
                })
            }

            row {
                cell(JBLabel(AnimatedIcon.Default())).visibleIf(object : ComponentPredicate() {
                    override fun invoke() = connectionStatus.value == ConnectionStatus.connecting
                    override fun addListener(listener: (Boolean) -> Unit) {
                        connectionStatus.addOnChangeListener { listener(it == ConnectionStatus.connecting) }
                    }
                })

                label("✔️ Successfully pinged server!").visibleIf(object : ComponentPredicate() {
                    override fun invoke() = connectionStatus.value == ConnectionStatus.connected
                    override fun addListener(listener: (Boolean) -> Unit) {
                        connectionStatus.addOnChangeListener { listener(it == ConnectionStatus.connected) }
                    }
                })

                label("❌ Failed to establish connection!").visibleIf(object : ComponentPredicate() {
                    override fun invoke() = connectionStatus.value == ConnectionStatus.failed
                    override fun addListener(listener: (Boolean) -> Unit) {
                        connectionStatus.addOnChangeListener { listener(it == ConnectionStatus.failed) }
                    }
                })

                val warningMessage: () -> String = {
                    "⚠️ Warning! Versions are different" +
                            if (serverVersion != null)
                                ": Server: $serverVersion Client: ${UTBotAllSettings.clientVersion}"
                            else ""
                }
                label(warningMessage()).visibleIf(
                    object : ComponentPredicate() {
                        override fun invoke() = connectionStatus.value == ConnectionStatus.warning
                        override fun addListener(listener: (Boolean) -> Unit) {
                            connectionStatus.addOnChangeListener {
                                listener(it == ConnectionStatus.warning)
                            }
                        }
                    }).applyToComponent {
                    connectionStatus.addOnChangeListener { newStatus ->
                        if (newStatus == ConnectionStatus.warning)
                            this.text = warningMessage()
                    }
                }
            }

            setupValidation()
        }.addToUI()

        addHtml("media/remote_path.html")
        panel {
            row {
                textField().bindText(settingsModel::remotePath).columns(COLUMNS_LARGE).applyToComponent {
                    remotePathTextField = this
                }.enabledIf(object : ComponentPredicate() {
                    override fun invoke() = !useDefaults.value
                    override fun addListener(listener: (Boolean) -> Unit) {
                        useDefaults.addOnChangeListener { newValue -> listener(!newValue) }
                    }
                })
            }
        }.addToUI()
    }
}
