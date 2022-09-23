package org.utbot.cpp.clion.plugin.ui.utbotToolWindow.targetToolWindow

import com.intellij.openapi.actionSystem.ActionGroup
import com.intellij.openapi.actionSystem.ActionManager
import com.intellij.openapi.actionSystem.ActionPlaces
import com.intellij.openapi.actionSystem.ActionToolbar
import com.intellij.openapi.actionSystem.DefaultActionGroup
import com.intellij.openapi.project.DumbAware
import com.intellij.openapi.ui.SimpleToolWindowPanel
import com.intellij.ui.CollectionListModel
import com.intellij.ui.ColoredListCellRenderer
import com.intellij.ui.SimpleTextAttributes
import com.intellij.ui.components.JBList
import com.intellij.ui.components.JBScrollPane
import javax.swing.JList
import javax.swing.ListSelectionModel
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.actions.RefreshTargetsAction

class UTBotTargetsToolWindow(
    listModel: CollectionListModel<UTBotTarget>,
    private val controller: UTBotTargetsController
): SimpleToolWindowPanel(true, true), DumbAware {
    private val uiList = JBList(listModel)

    init {
        toolbar = createActionToolBar().let {
            it.targetComponent = this.component
            it.component
        }

        uiList.cellRenderer = Renderer()
        uiList.selectionMode = ListSelectionModel.SINGLE_SELECTION
        uiList.addListSelectionListener {
            if (!uiList.isSelectionEmpty)
                controller.selectionChanged(uiList.selectedValue)
        }
        uiList.setEmptyText(UTBot.message("targets.notargets.description"))

        val panel = JBScrollPane()
        panel.setViewportView(uiList)
        setContent(panel)
    }

    fun setBusy(busy: Boolean) {
        uiList.setPaintBusy(busy)
    }

    fun setSelectedTarget(utBotTarget: UTBotTarget) {
        uiList.setSelectedValue(utBotTarget, true)
    }

    private fun createActionToolBar(isHorizontal: Boolean = false): ActionToolbar {
        return ActionManager.getInstance().createActionToolbar(ActionPlaces.TOOLBAR, createActionGroup(), isHorizontal)
    }

    private fun createActionGroup(): ActionGroup {
        return DefaultActionGroup(RefreshTargetsAction())
    }

    private inner class Renderer : ColoredListCellRenderer<UTBotTarget>() {
        override fun customizeCellRenderer(
            list: JList<out UTBotTarget>,
            target: UTBotTarget,
            index: Int,
            selected: Boolean,
            hasFocus: Boolean
        ) {
            append(target.name, SimpleTextAttributes.REGULAR_BOLD_ATTRIBUTES)
            append(" ")
            append(target.description, SimpleTextAttributes.GRAYED_ATTRIBUTES)
        }
    }
}
