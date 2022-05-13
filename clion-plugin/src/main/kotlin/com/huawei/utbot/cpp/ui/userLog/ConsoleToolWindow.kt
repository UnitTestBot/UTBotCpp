package com.huawei.utbot.cpp.ui.userLog

import com.huawei.utbot.cpp.utils.getClient
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
import org.tinylog.Level
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

class UTBotConsole(project: Project) : ConsoleViewImpl(project, true) {
    fun info(message: String) {
        print(message, ConsoleViewContentType.NORMAL_OUTPUT)
    }
}

enum class OutputType(val title: String) {
    SERVER_LOG("Server log"), CLIENT_LOG("Client log"), GTEST("GTest log")
}

@Service
class OutputWindowProvider(val project: Project) : Disposable {
    val clientOutputChannel: OutputChannel by lazy { createOutputChannel(OutputType.CLIENT_LOG) }
    val gtestOutputChannel: OutputChannel by lazy { createOutputChannel(OutputType.GTEST) }
    val serverOutputChannel: OutputChannel by lazy { createOutputChannel(OutputType.SERVER_LOG) }

    data class OutputChannel(val uiComponent: Component, val outputConsole: UTBotConsole, val type: OutputType)

    private fun createClientLogOutputWindow(): OutputChannel {
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
                    ComboBox(Level.values().map { it.name }.toTypedArray()).apply {
                        addItemListener { itemEvent ->
                            if (itemEvent.stateChange == ItemEvent.SELECTED) {
                                project.getClient().setLoggingLevel(
                                    Level.values().find { it.name == (itemEvent.item as String) }!!
                                )
                            }
                        }
                    },
                )
            }, BorderLayout.WEST)
        }

        return OutputChannel(toolWindowPanel, console, OutputType.CLIENT_LOG)
    }

    override fun dispose() {}

    private fun createOutputChannel(type: OutputType): OutputChannel {
        return when (type) {
            OutputType.CLIENT_LOG -> createClientLogOutputWindow()
            else -> {
                UTBotConsole(project).let {
                    Disposer.register(this@OutputWindowProvider, it)
                    OutputChannel(it.component, it, type)
                }
            }
        }
    }
}

class ConsoleToolWindow(val project: Project) : SimpleToolWindowPanel(true, true) {
    private var mainUI: JBTabbedPane = JBTabbedPane()

    init {
        mainUI.tabComponentInsets = Insets(0, 0, 0, 0)

        val provider = project.service<OutputWindowProvider>()
        for (channel in listOf(
            provider.clientOutputChannel,
            provider.gtestOutputChannel,
            provider.serverOutputChannel
        )) {
            mainUI.addTab(channel.type.title, channel.uiComponent)
        }

        setContent(mainUI)
    }
}
