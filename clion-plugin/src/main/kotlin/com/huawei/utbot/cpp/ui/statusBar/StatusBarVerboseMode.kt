package com.huawei.utbot.cpp.ui.statusBar

import com.huawei.utbot.cpp.utils.generatorSettings
import com.intellij.openapi.project.Project
import com.intellij.openapi.wm.StatusBar
import com.intellij.openapi.wm.StatusBarWidget
import com.intellij.openapi.wm.StatusBarWidgetFactory
import com.intellij.util.Consumer
import java.awt.Component
import java.awt.event.MouseEvent

class VerboseModeWidgetFactory : StatusBarWidgetFactory {
    override fun getId() = STATUS_BAR_ID

    override fun getDisplayName() = STATUS_BAR_DISPLAY_NAME

    override fun isAvailable(p0: Project): Boolean = true

    override fun createWidget(p0: Project): StatusBarWidget {
        return UTBotStatusBarVerboseWidget()
    }

    override fun disposeWidget(p0: StatusBarWidget) {}

    override fun canBeEnabledOn(p0: StatusBar): Boolean {
        return p0.project != null
    }

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
    }

    override fun dispose() {}

    override fun getTooltipText() = VerboseModeWidgetFactory.STATUS_BAR_DISPLAY_NAME

    override fun getClickConsumer() = Consumer<MouseEvent> { _ ->
        val settings = statusBar?.project!!.generatorSettings
        settings.verbose = !settings.verbose
        statusBar?.updateWidget(ID())
    }

    override fun getText(): String {
        if (statusBar == null)
            return ""
        return if (statusBar?.project!!.generatorSettings.verbose) "✔ UTBot: verbose formatting" else "❌ UTBot: verbose formatting"
    }

    override fun getAlignment(): Float = Component.CENTER_ALIGNMENT

    override fun getPresentation(): StatusBarWidget.WidgetPresentation = this

    companion object {
        val WIDGET_ID: String = UTBotStatusBarVerboseWidget::class.java.name
    }
}
