package com.huawei.utbot.cpp.ui.wizard

import com.intellij.ide.wizard.Step
import com.intellij.openapi.ui.DialogPanel
import com.intellij.util.ui.HtmlPanel
import javax.swing.BoxLayout
import javax.swing.Icon
import javax.swing.JComponent
import javax.swing.JPanel
import java.awt.Component
import java.awt.Dimension

abstract class UTBotWizardStep : Step {
    protected val panel by lazy { JPanel() }
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

    override fun _commit(finishChosen: Boolean) {
        onApplyCallbacks.forEach {
            it.invoke()
        }
    }

    override fun getIcon(): Icon? {
        return null
    }

    override fun getComponent(): JComponent {
        return panel
    }

    override fun getPreferredFocusedComponent(): JComponent? {
        return panel
    }

    private fun getTextResource(resource: String): String {
        return this.javaClass.classLoader.getResource(resource)?.readText()
            ?: error("Unable to get resource: $resource")
    }

    fun addHtml(htmlResource: String) {
        panel.add(createHtmlComponent(getTextResource(htmlResource)))
    }

    open fun canProceedToNextStep(): Boolean = true

    private fun createHtmlComponent(html: String): JComponent {
        return object : HtmlPanel() {
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
}
