package org.utbot.cpp.clion.plugin.ui.statusBar

import com.intellij.ide.DataManager
import com.intellij.openapi.actionSystem.ActionPlaces
import com.intellij.openapi.actionSystem.AnAction
import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.DataContext
import com.intellij.openapi.actionSystem.DefaultActionGroup
import com.intellij.openapi.options.ShowSettingsUtil
import com.intellij.openapi.project.DumbAwareAction
import com.intellij.openapi.project.Project
import com.intellij.openapi.ui.popup.JBPopupFactory
import com.intellij.openapi.ui.popup.ListPopup
import com.intellij.openapi.wm.StatusBar
import com.intellij.openapi.wm.StatusBarWidget
import com.intellij.openapi.wm.StatusBarWidgetFactory
import com.intellij.ui.awt.RelativePoint
import com.intellij.util.Consumer
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.actions.ChangeVerboseModeAction
import org.utbot.cpp.clion.plugin.actions.ReconnectAction
import org.utbot.cpp.clion.plugin.actions.configure.ConfigureProjectAction
import org.utbot.cpp.clion.plugin.actions.configure.ReconfigureProjectAction
import org.utbot.cpp.clion.plugin.actions.ShowWizardAction
import org.utbot.cpp.clion.plugin.actions.SyncWrappersAndStubsAction
import org.utbot.cpp.clion.plugin.actions.TogglePluginAction
import org.utbot.cpp.clion.plugin.client.ManagedClient
import org.utbot.cpp.clion.plugin.listeners.ConnectionStatus
import org.utbot.cpp.clion.plugin.listeners.PluginActivationListener
import org.utbot.cpp.clion.plugin.listeners.UTBotEventsListener
import org.utbot.cpp.clion.plugin.settings.UTBotConfigurable
import org.utbot.cpp.clion.plugin.settings.settings
import org.utbot.cpp.clion.plugin.utils.client
import java.awt.Component
import java.awt.Point
import java.awt.event.MouseEvent


class ConnectionStatusBarWidgetFactory : StatusBarWidgetFactory {
    override fun getId() = UTBotStatusBarWidget.WIDGET_ID

    override fun getDisplayName() = STATUS_BAR_DISPLAY_NAME

    override fun isAvailable(project: Project): Boolean = true

    override fun createWidget(project: Project): StatusBarWidget = UTBotStatusBarWidget(project)

    override fun disposeWidget(widget: StatusBarWidget) {
        (widget as? UTBotStatusBarWidget)?.dispose()
    }

    override fun canBeEnabledOn(statusBar: StatusBar): Boolean = statusBar.project != null

    companion object {
        const val STATUS_BAR_DISPLAY_NAME = "UTBot"
    }
}

class UTBotStatusBarWidget(val project: Project) : StatusBarWidget, StatusBarWidget.TextPresentation {
    private var statusBar: StatusBar? = null
    private val client: ManagedClient = project.client
    private val myStatusText: String get() = if (project.settings.storedSettings.isPluginEnabled) client.connectionStatus.description else "disabled"

    init {
        project.messageBus.connect(this).subscribe(
            UTBotEventsListener.CONNECTION_CHANGED_TOPIC,
            object : UTBotEventsListener {
                override fun onConnectionChange(oldStatus: ConnectionStatus, newStatus: ConnectionStatus) {
                    statusBar?.updateWidget(ID())
                }
            })
        project.messageBus.connect(this).subscribe(PluginActivationListener.TOPIC, PluginActivationListener {
            statusBar?.updateWidget(ID())
        })
    }

    override fun ID(): String = WIDGET_ID

    override fun install(statusbar: StatusBar) {
        this.statusBar = statusbar
    }

    override fun dispose() {}

    override fun getTooltipText() = "UTBot: Connection status or 'disabled' if plugin is disabled"

    override fun getClickConsumer() = Consumer<MouseEvent> { event ->
        val component = event.component

        val popup = StatusBarActionsPopup.getPopup(DataManager.getInstance().getDataContext(component))
        val dimension = popup.content.preferredSize
        // the point for popup was set experimentally
        val popupLocation = Point(0, -dimension.height)

        popup.show(RelativePoint(component, popupLocation))
    }

    override fun getText(): String = "UTBot: $myStatusText"

    override fun getAlignment(): Float = Component.CENTER_ALIGNMENT

    override fun getPresentation(): StatusBarWidget.WidgetPresentation = this

    companion object {
        val WIDGET_ID: String = UTBotStatusBarWidget::class.java.name
    }
}

object StatusBarActionsPopup {
    fun getPopup(dataContext: DataContext): ListPopup {
        val actions = getActions()

        return JBPopupFactory.getInstance()
            .createActionGroupPopup(
                ConnectionStatusBarWidgetFactory.STATUS_BAR_DISPLAY_NAME, actions,
                dataContext, JBPopupFactory.ActionSelectionAid.SPEEDSEARCH, false,
                ActionPlaces.POPUP
            )
    }
    private object ShortcutSettingsAction : DumbAwareAction(UTBot.message("actions.settings.text")) {
        override fun actionPerformed(e: AnActionEvent) {
            ShowSettingsUtil.getInstance().showSettingsDialog(e.project, UTBotConfigurable::class.java)
        }

        override fun update(e: AnActionEvent) {
            super.update(e)
            e.presentation.isEnabledAndVisible = e.project != null
        }
    }

    private fun getActions(): DefaultActionGroup {
        val actionGroup = DefaultActionGroup()
        actionGroup.isPopup = true
        fun addToActionGroup(action: AnAction, isLast: Boolean = false) {
            actionGroup.add(action)
            if (!isLast)
                actionGroup.addSeparator()
        }

        addToActionGroup(ShowWizardAction())
        addToActionGroup(ConfigureProjectAction())
        addToActionGroup(ReconfigureProjectAction())
        addToActionGroup(ShortcutSettingsAction)
        addToActionGroup(ReconnectAction())
        addToActionGroup(ChangeVerboseModeAction())
        addToActionGroup(TogglePluginAction(), isLast = true)

        return actionGroup
    }
}
