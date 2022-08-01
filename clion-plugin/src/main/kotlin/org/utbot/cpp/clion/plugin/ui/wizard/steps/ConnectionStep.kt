@file:Suppress("UnstableApiUsage")

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
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.launch
import org.utbot.cpp.clion.plugin.client.GrpcClient
import org.utbot.cpp.clion.plugin.grpc.getVersionGrpcRequest
import org.utbot.cpp.clion.plugin.settings.UTBotAllProjectSettings
import org.utbot.cpp.clion.plugin.settings.UTBotSettingsModel
import org.utbot.cpp.clion.plugin.settings.settings
import org.utbot.cpp.clion.plugin.ui.wizard.UTBotBaseWizardStep
import org.utbot.cpp.clion.plugin.utils.toWslFormat
import org.utbot.cpp.clion.plugin.utils.validateInput
import javax.swing.JComponent
import javax.swing.event.DocumentEvent
import kotlin.properties.Delegates
import org.utbot.cpp.clion.plugin.settings.UTBotProjectStoredSettings
import org.utbot.cpp.clion.plugin.utils.isWindows

enum class ConnectionStatus {
    Connected,
    Connecting,
    Failed,
    Suspicious,
}

class ConnectionStep(
    private val project: Project,
    private val settingsModel: UTBotSettingsModel,
    private val parentDisposable: Disposable,
) : UTBotBaseWizardStep() {
    private lateinit var hostTextField: JBTextField
    private lateinit var portTextField: JBTextField
    private lateinit var remotePathTextField: JBTextField

    private var serverVersion: String? = null

    private val connectionStatus = ObservableValue(ConnectionStatus.Failed)
    private val useConnectionDefaults = ObservableValue(false)

    init {
        useConnectionDefaults.addOnChangeListener { newValue ->
            if (newValue) {
                portTextField.text = UTBotAllProjectSettings.DEFAULT_PORT.toString()
                hostTextField.text = UTBotAllProjectSettings.DEFAULT_HOST
                remotePathTextField.text = if (isWindows) project.settings.projectPath.toWslFormat()
                    else UTBotProjectStoredSettings.REMOTE_PATH_VALUE_FOR_LOCAL_SCENARIO
            }
        }
    }

    override fun _init() {
        super._init()
        pingServer()
    }

    override fun canProceedToNextStep(): Boolean {
        if (connectionStatus.value != ConnectionStatus.Failed) {
            return true
        }

        return NotConnectedWarningDialog(project).showAndGet()
    }

    override fun createUI() {
        addHtml("media/connection.html")
        panel {
            row {
                checkBox("Default server configuration on localhost (or WSL2):")
                    .bindSelected(getter = { useConnectionDefaults.value }, setter = { newValue ->
                        useConnectionDefaults.value = newValue
                    }).selected.addListener { newValue ->
                        useConnectionDefaults.value = newValue
                    }
            }

            row("Host") {
                textField().also {
                    it.bindText(settingsModel.globalSettings::serverName)
                    hostTextField = it.component
                }.columns(COLUMNS_MEDIUM).enabledIf(object : ComponentPredicate() {
                    override fun invoke() = !useConnectionDefaults.value
                    override fun addListener(listener: (Boolean) -> Unit) {
                        useConnectionDefaults.addOnChangeListener { newValue -> listener(!newValue) }
                    }
                })
            }

            row("Port") {
                intTextField(
                    0..65535,
                    1
                ).also {
                    it.bindIntText(settingsModel.globalSettings::port)
                }.columns(COLUMNS_MEDIUM).applyToComponent {
                    portTextField = this
                }.enabledIf(object : ComponentPredicate() {
                    override fun invoke() = !useConnectionDefaults.value
                    override fun addListener(listener: (Boolean) -> Unit) {
                        useConnectionDefaults.addOnChangeListener { newValue -> listener(!newValue) }
                    }
                })
            }

            row {
                cell(JBLabel(AnimatedIcon.Default())).visibleIf(object : ComponentPredicate() {
                    override fun invoke() = connectionStatus.value == ConnectionStatus.Connecting
                    override fun addListener(listener: (Boolean) -> Unit) {
                        connectionStatus.addOnChangeListener { listener(it == ConnectionStatus.Connecting) }
                    }
                })

                label("✔️ Successfully pinged server!").visibleIf(object : ComponentPredicate() {
                    override fun invoke() = connectionStatus.value == ConnectionStatus.Connected
                    override fun addListener(listener: (Boolean) -> Unit) {
                        connectionStatus.addOnChangeListener { listener(it == ConnectionStatus.Connected) }
                    }
                })

                label("❌ Failed to establish connection!").visibleIf(object : ComponentPredicate() {
                    override fun invoke() = connectionStatus.value == ConnectionStatus.Failed
                    override fun addListener(listener: (Boolean) -> Unit) {
                        connectionStatus.addOnChangeListener { listener(it == ConnectionStatus.Failed) }
                    }
                })

                val warningMessage: () -> String = {
                    "⚠️ Warning! Versions are different or not defined:" +
                            "Client: ${UTBotAllProjectSettings.clientVersion} Server: ${serverVersion ?: "not defined"}"
                }
                label(warningMessage()).visibleIf(
                    object : ComponentPredicate() {
                        override fun invoke() = connectionStatus.value == ConnectionStatus.Suspicious
                        override fun addListener(listener: (Boolean) -> Unit) {
                            connectionStatus.addOnChangeListener {
                                listener(it == ConnectionStatus.Suspicious)
                            }
                        }
                    }).applyToComponent {
                    connectionStatus.addOnChangeListener { newStatus ->
                        if (newStatus == ConnectionStatus.Suspicious)
                            this.text = warningMessage()
                    }
                }
            }

            setupValidation()
        }.addToUI()

        addHtml("media/remote_path.html")
        panel {
            row {
                textField().bindText(settingsModel.projectSettings::remotePath).columns(COLUMNS_LARGE)
                    .applyToComponent {
                        remotePathTextField = this
                    }.enabledIf(object : ComponentPredicate() {
                    override fun invoke() = !useConnectionDefaults.value
                    override fun addListener(listener: (Boolean) -> Unit) {
                        useConnectionDefaults.addOnChangeListener { newValue -> listener(!newValue) }
                    }
                })
            }
        }.addToUI()
    }

    private suspend fun pingServer(port: Int, host: String): ConnectionStatus {
        connectionStatus.value = ConnectionStatus.Connecting
        runCatching {
            GrpcClient(port, host, "DummyId").use { client ->
                serverVersion = client.stub.handshake(getVersionGrpcRequest()).version

                if (serverVersion != UTBotAllProjectSettings.clientVersion)
                    return ConnectionStatus.Suspicious
                return ConnectionStatus.Connected
            }
        }.getOrElse { exception ->
            when (exception) {
                is io.grpc.StatusException -> return ConnectionStatus.Failed
                else -> {
                    connectionStatus.value = ConnectionStatus.Failed
                    throw exception
                }
            }
        }
    }

    private fun pingServer() {
        CoroutineScope(Dispatchers.IO + SupervisorJob()).launch {
            connectionStatus.value = pingServer(portTextField.text.toInt(), hostTextField.text)
        }
    }

    private fun setupValidation() {
        portTextField.validateInput(parentDisposable) {
            val value = portTextField.text.toUShortOrNull()
            if (value == null) {
                connectionStatus.value = ConnectionStatus.Failed
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
}

internal class ObservableValue<T>(initialValue: T) {
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

internal class NotConnectedWarningDialog(project: Project) : DialogWrapper(project) {
    init {
        title = "❌ Server is unreachable!"
        super.init()
    }

    override fun createCenterPanel(): JComponent = panel {
        row {
            text(
                """UTBot failed to establish connection with specified server. 
                   If you wish to continue anyway, press "Ok" button.
                """.trimMargin(),
                MAX_LINE_LENGTH
            )
        }
        row {
            text(
                """In any case, you will need to specify correct port and host of UTBot server to use the plugin.
                   You can do it via CLion Settings -> Tools -> UTBot Settings
                """.trimIndent(),
                MAX_LINE_LENGTH
            )
        }
    }

    companion object {
        const val MAX_LINE_LENGTH = 100
    }
}
