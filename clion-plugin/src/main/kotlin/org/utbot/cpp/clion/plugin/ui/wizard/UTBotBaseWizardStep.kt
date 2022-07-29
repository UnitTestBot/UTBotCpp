package org.utbot.cpp.clion.plugin.ui.wizard

import com.intellij.ide.wizard.Step
import com.intellij.openapi.ui.DialogPanel
import com.intellij.util.ui.HtmlPanel
import javax.swing.BoxLayout
import javax.swing.Icon
import javax.swing.JComponent
import javax.swing.JPanel
import java.awt.Component
import java.awt.Dimension

abstract class UTBotBaseWizardStep : Step {
    private val panel by lazy { JPanel() }
    private var isInitialized = false

    private val onApplyCallbacks = mutableListOf<() -> Unit>()

    abstract fun createUI()

    override fun _init() {
        if (!isInitialized) {
            panel.layout = BoxLayout(panel, BoxLayout.Y_AXIS)
            panel.preferredSize = Dimension(800, 400)
            panel.minimumSize = panel.preferredSize
            createUI()

            isInitialized = true
        }
    }

    fun DialogPanel.addToUI() {
        alignmentX = Component.LEFT_ALIGNMENT
        panel.add(this)
        onApplyCallbacks.add { apply() }
    }

    override fun _commit(finishChosen: Boolean) = onApplyCallbacks.forEach { it.invoke() }

    //TODO: find a good icon and use here
    override fun getIcon(): Icon? = null

    override fun getComponent(): JComponent = panel

    override fun getPreferredFocusedComponent(): JComponent? = panel

    fun addHtml(htmlResource: String) {
        val text = this.javaClass.classLoader.getResource(htmlResource)?.readText()
            ?: error("Unable to get resource: $htmlResource")
        panel.add(createHtmlComponent(text))
    }

    open fun canProceedToNextStep(): Boolean = true

    private fun createHtmlComponent(html: String): JComponent = object : HtmlPanel() {
        init {
            update()
            alignmentX = Component.LEFT_ALIGNMENT
            adjustHeightToTextHeight()
        }

        override fun getBody() = html

        fun adjustHeightToTextHeight() {
            // set dummy size, to update preferred
            size = Dimension(100, Short.MAX_VALUE.toInt())
            size = preferredSize
            minimumSize = preferredSize
            maximumSize = preferredSize
            update()
        }
    }
}
