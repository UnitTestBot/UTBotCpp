package org.utbot.cpp.clion.plugin.ui.services

import com.intellij.openapi.Disposable
import com.intellij.openapi.components.Service
import com.intellij.openapi.project.Project
import com.intellij.openapi.ui.ComboBox
import com.intellij.openapi.ui.SimpleToolWindowPanel
import com.intellij.openapi.util.Disposer
import com.intellij.util.ui.JBUI
import com.intellij.util.ui.components.BorderLayoutPanel
import org.utbot.cpp.clion.plugin.client.logger.LogLevel
import org.utbot.cpp.clion.plugin.ui.userLog.OutputType
import org.utbot.cpp.clion.plugin.ui.userLog.UTBotConsole
import org.utbot.cpp.clion.plugin.utils.logger
import java.awt.BorderLayout
import java.awt.Component
import java.awt.GridLayout
import java.awt.event.ItemEvent
import javax.swing.JPanel

@Service
class OutputProvider(val project: Project) : Disposable {
    val clientOutputChannel: OutputChannel by lazy { createOutputChannel(OutputType.CLIENT_LOG) }
    val gTestOutputChannel: OutputChannel by lazy { createOutputChannel(OutputType.GTEST_LOG) }
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
                    ComboBox(LogLevel.values().map { it.name }.toTypedArray()).apply {
                        addItemListener { itemEvent ->
                            if (itemEvent.stateChange == ItemEvent.SELECTED) {
                                project.logger.level = LogLevel.values().find { it.name == (itemEvent.item as String) }!!
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
                    Disposer.register(this@OutputProvider, it)
                    OutputChannel(it.component, it, type)
                }
            }
        }
    }
}