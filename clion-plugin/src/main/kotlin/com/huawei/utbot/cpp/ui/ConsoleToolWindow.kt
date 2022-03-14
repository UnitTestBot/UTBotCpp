package com.huawei.utbot.cpp.ui

import com.huawei.utbot.cpp.messaging.ConnectionStatus
import com.huawei.utbot.cpp.messaging.UTBotEventsListener
import com.huawei.utbot.cpp.client.Client
import com.huawei.utbot.cpp.client.LogLevel
import com.intellij.execution.impl.ConsoleViewImpl
import com.intellij.execution.ui.ConsoleViewContentType
import com.intellij.openapi.Disposable
import com.intellij.openapi.components.Service
import com.intellij.openapi.components.service
import com.intellij.openapi.diagnostic.Logger
import com.intellij.openapi.project.Project
import com.intellij.openapi.ui.ComboBox
import com.intellij.openapi.ui.SimpleToolWindowPanel
import com.intellij.openapi.util.Disposer
import com.intellij.openapi.wm.ToolWindow
import com.intellij.openapi.wm.ToolWindowFactory
import com.intellij.ui.components.JBTabbedPane
import com.intellij.util.ui.JBUI
import com.intellij.util.ui.components.BorderLayoutPanel
import javax.swing.JPanel
import java.awt.BorderLayout
import java.awt.Component
import java.awt.GridLayout
import java.awt.Insets
import java.awt.event.ItemEvent

class ConsoleToolWindowProvider : ToolWindowFactory {
    private val logger = Logger.getInstance(this::class.java)

    override fun createToolWindowContent(project: Project, toolWindow: ToolWindow) {
        logger.debug("createToolWindowContent was called")
        val contentManager = toolWindow.contentManager
        val content =
            contentManager.factory.createContent(ConsoleToolWindow(project), null, false)
        toolWindow.contentManager.addContent(content)
    }
}

class UTBotConsole(project: Project) : ConsoleViewImpl(project, true)

enum class OutputType(val id: String) {
    SERVER_LOG("Server log"), CLIENT_LOG("Client log"), GTEST("GTest log")
}

@Service
class OutputWindowProvider(val project: Project): Disposable {
    var outputs: MutableMap<OutputType, UTBotConsole>
    val outputUI: MutableMap<OutputType, Component>

    data class OutputChannel(val uiComponent: Component, val output: UTBotConsole)

    init {
        val outputChannels = OutputType.values().map { type -> type to createOutputChannel(type, project) }
        outputUI = outputChannels.associate { it.first to it.second.uiComponent }.toMutableMap()
        outputs = outputChannels.associate { it.first to it.second.output }.toMutableMap()
    }

    private fun createServerLogOutputWindow(project: Project): OutputChannel {
        val toolWindowPanel = SimpleToolWindowPanel(true, true)
        val console = UTBotConsole(project)
        Disposer.register(this, console)
        toolWindowPanel.setContent(console.component)
        toolWindowPanel.toolbar = BorderLayoutPanel().apply {
            border = JBUI.Borders.empty()
            withPreferredHeight(JBUI.scale(30))
            withMinimumHeight(JBUI.scale(30))
            add(JPanel().apply {
                layout = GridLayout(1, components.size)
                add(
                    ComboBox(LogLevel.values().map { it.id }.toTypedArray()).apply {

                        project.messageBus.connect().subscribe(UTBotEventsListener.CONNECTION_CHANGED_TOPIC,
                            object : UTBotEventsListener {
                                override fun onConnectionChange(
                                    oldStatus: ConnectionStatus,
                                    newStatus: ConnectionStatus
                                ) {
                                    isEnabled = newStatus == ConnectionStatus.CONNECTED
                                }
                            })

                        addItemListener { itemEvent ->
                            if (itemEvent.stateChange == ItemEvent.SELECTED) {
                                project.service<Client>().setLoggingLevel(
                                    LogLevel.values().find { it.id == (itemEvent.item as String) }!!
                                )
                            }
                        }
                    },
                )
            }, BorderLayout.WEST)
        }

        return OutputChannel(toolWindowPanel, console)
    }

    override fun dispose() {}

    private fun createOutputChannel(type: OutputType, project: Project): OutputChannel {
        return when (type) {
            OutputType.SERVER_LOG -> createServerLogOutputWindow(project)
            else -> {
                UTBotConsole(project).let {
                    Disposer.register(this@OutputWindowProvider, it)
                    OutputChannel(it.component, it)
                }
            }
        }
    }
}

class ConsoleToolWindow(val project: Project) : SimpleToolWindowPanel(true, true) {
    private var mainUI: JBTabbedPane = JBTabbedPane()

    init {
        mainUI.tabComponentInsets = Insets(0, 0, 0, 0)

        val factory = project.service<OutputWindowProvider>()
        for (type in OutputType.values()) {
            mainUI.addTab(type.id, factory.outputUI[type])
        }

        setContent(mainUI)
    }
}
