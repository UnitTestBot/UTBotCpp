package org.utbot.cpp.clion.plugin.ui.targetsToolWindow

import com.intellij.openapi.ui.SimpleToolWindowPanel
import com.intellij.ui.CollectionListModel
import com.intellij.ui.ColoredListCellRenderer
import com.intellij.ui.SimpleTextAttributes
import com.intellij.ui.components.JBList
import com.intellij.ui.components.JBScrollPane
import javax.swing.JList
import javax.swing.ListSelectionModel

class UTBotTargetsToolWindow(
    listModel: CollectionListModel<UTBotTarget>,
    private val controller: UTBotTargetsController,
): SimpleToolWindowPanel(true, true) {
    private val uiList = JBList(listModel)

    init {
        val panel = JBScrollPane()
        uiList.cellRenderer = Renderer()
        uiList.selectionMode = ListSelectionModel.SINGLE_SELECTION
        uiList.addListSelectionListener {
            controller.selectionChanged(uiList.selectedValue)
        }
        panel.setViewportView(uiList)
        setContent(panel)
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
