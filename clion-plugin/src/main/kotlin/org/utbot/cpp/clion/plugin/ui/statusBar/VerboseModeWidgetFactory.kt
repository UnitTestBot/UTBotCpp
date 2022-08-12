package org.utbot.cpp.clion.plugin.ui.statusBar

import com.intellij.openapi.project.Project
import com.intellij.openapi.wm.StatusBar
import com.intellij.openapi.wm.StatusBarWidget
import com.intellij.openapi.wm.StatusBarWidgetFactory

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