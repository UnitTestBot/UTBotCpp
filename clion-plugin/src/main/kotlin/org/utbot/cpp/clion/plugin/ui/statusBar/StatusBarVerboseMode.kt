package org.utbot.cpp.clion.plugin.ui.statusBar

import com.intellij.openapi.project.Project
import com.intellij.openapi.wm.StatusBar
import com.intellij.openapi.wm.StatusBarWidget
import com.intellij.openapi.wm.StatusBarWidgetFactory
import com.intellij.util.Consumer
import org.utbot.cpp.clion.plugin.listeners.UTBotSettingsChangedListener
import org.utbot.cpp.clion.plugin.settings.settings
import java.awt.Component
import java.awt.event.MouseEvent

class VerboseModeWidgetFactory : StatusBarWidgetFactory {
    override fun getId() = STATUS_BAR_ID

    override fun getDisplayName() = STATUS_BAR_DISPLAY_NAME

    override fun isAvailable(project: Project): Boolean = true

    override fun createWidget(project: Project): StatusBarWidget = UTBotStatusBarVerboseWidget()

    override fun disposeWidget(widget: StatusBarWidget) {}

    override fun canBeEnabledOn(statusBar: StatusBar): Boolean = statusBar.project != null

    companion object {
        const val STATUS_BAR_ID = "UTBot: verbose mode"
        const val STATUS_BAR_DISPLAY_NAME = "UTBot: Verbose Formatting"
    }
}

class UTBotStatusBarVerboseWidget : StatusBarWidget, StatusBarWidget.TextPresentation {
    private var statusBar: StatusBar? = null

    override fun ID(): String = WIDGET_ID

    override fun install(statusbar: StatusBar) {
        this.statusBar = statusbar
        statusBar?.updateWidget(ID())
        statusbar.project?.messageBus?.connect()?.subscribe(UTBotSettingsChangedListener.TOPIC, UTBotSettingsChangedListener {
            statusbar.updateWidget(ID())
        })
    }

    override fun dispose() {}

    override fun getTooltipText() = VerboseModeWidgetFactory.STATUS_BAR_DISPLAY_NAME

    override fun getClickConsumer() = Consumer<MouseEvent> { _ ->
        val settings = statusBar?.project!!.settings.storedSettings
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
