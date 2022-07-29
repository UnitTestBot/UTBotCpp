package org.utbot.cpp.clion.plugin.ui.userLog

import com.intellij.execution.impl.ConsoleViewImpl
import com.intellij.execution.ui.ConsoleViewContentType
import com.intellij.openapi.components.service
import com.intellij.openapi.project.Project
import com.intellij.openapi.ui.SimpleToolWindowPanel
import com.intellij.ui.components.JBTabbedPane
import org.utbot.cpp.clion.plugin.ui.services.OutputProvider
import java.awt.Insets


class UTBotConsole(project: Project) : ConsoleViewImpl(project, true) {
    fun info(message: String) = print(message, ConsoleViewContentType.NORMAL_OUTPUT)
}

enum class OutputType(val title: String) {
    SERVER_LOG("Server log"),
    CLIENT_LOG("Client log"),
    GTEST_LOG("GTest log"),
}

class ConsoleToolWindow(val project: Project) : SimpleToolWindowPanel(true, true) {
    private var mainUI: JBTabbedPane = JBTabbedPane()

    init {
        mainUI.tabComponentInsets = Insets(0, 0, 0, 0)

        val provider = project.service<OutputProvider>()
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
