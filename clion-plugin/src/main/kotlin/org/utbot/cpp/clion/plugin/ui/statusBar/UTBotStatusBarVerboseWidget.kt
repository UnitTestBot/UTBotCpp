package org.utbot.cpp.clion.plugin.ui.statusBar

import com.intellij.openapi.components.service
import com.intellij.openapi.wm.StatusBar
import com.intellij.openapi.wm.StatusBarWidget
import com.intellij.util.Consumer
import org.utbot.cpp.clion.plugin.client.ClientManager
import org.utbot.cpp.clion.plugin.listeners.UTBotSettingsChangedListener
import org.utbot.cpp.clion.plugin.settings.settings
import java.awt.Component
import java.awt.event.MouseEvent

class UTBotStatusBarVerboseWidget : StatusBarWidget, StatusBarWidget.TextPresentation {
    private var statusBar: StatusBar? = null

    override fun ID(): String = WIDGET_ID

    override fun install(statusbar: StatusBar) {
        this.statusBar = statusbar
        statusBar?.updateWidget(ID())
        statusbar.project?.let { project ->
            project.messageBus.connect(project.service<ClientManager>()).subscribe(UTBotSettingsChangedListener.TOPIC, UTBotSettingsChangedListener {
                statusbar.updateWidget(ID())
            })
        }
    }

    override fun dispose() {}

    override fun getTooltipText() = VerboseModeWidgetFactory.STATUS_BAR_DISPLAY_NAME

    override fun getClickConsumer() = Consumer<MouseEvent> { _ ->
        val project = statusBar?.project ?: return@Consumer
        val settings = project.settings.storedSettings
        settings.verbose = !settings.verbose
        statusBar?.updateWidget(ID())
    }

    override fun getText(): String {
        val project = statusBar?.project ?: return ""
        return if (project.settings.storedSettings.verbose) "✔ UTBot: verbose formatting" else "❌ UTBot: verbose formatting"
    }


    override fun getAlignment(): Float = Component.CENTER_ALIGNMENT

    override fun getPresentation(): StatusBarWidget.WidgetPresentation = this

    companion object {
        val WIDGET_ID: String = UTBotStatusBarVerboseWidget::class.java.name
    }
}
