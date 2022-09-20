package org.utbot.cpp.clion.plugin.ui.wizard

import com.intellij.ide.wizard.Step
import com.intellij.openapi.Disposable
import com.intellij.openapi.ui.DialogPanel
import com.intellij.ui.components.JBTextField
import com.intellij.ui.dsl.builder.Cell
import com.intellij.util.ui.HtmlPanel
import com.intellij.util.ui.UIUtil
import javax.swing.Box
import javax.swing.BoxLayout
import javax.swing.Icon
import javax.swing.JComponent
import javax.swing.JPanel
import org.utbot.cpp.clion.plugin.utils.ComponentValidationInfo
import org.utbot.cpp.clion.plugin.utils.ValidationCondition
import org.utbot.cpp.clion.plugin.utils.validateInput
import java.awt.Component
import java.awt.Dimension
import java.awt.Font

abstract class UTBotBaseWizardStep(private val parentDisposable: Disposable) : Step {
    private val panel by lazy { JPanel() }
    private var isInitialized = false
    private val validators = mutableListOf<ComponentValidationInfo>()
    private val onApplyCallbacks = mutableListOf<() -> Unit>()
    class Validator(val component: JComponent, val isValid: ()->Boolean)
    abstract fun createUI()

    override fun _init() {
        if (!isInitialized) {
            panel.layout = BoxLayout(panel, BoxLayout.Y_AXIS)
            panel.preferredSize = Dimension(STEP_WIDTH, STEP_HEIGHT)
            panel.minimumSize = panel.preferredSize
            createUI()
            panel.add(Box.createVerticalGlue())

            isInitialized = true
        }
    }

    protected fun DialogPanel.addToUI() {
        this.maximumSize = Dimension(maximumSize.width, minimumSize.height)
        onApplyCallbacks.add { apply() }
        addComponentToStep(this)
    }

    protected fun <T : JBTextField> Cell<T>.validateWith(vararg conditions: ValidationCondition): Cell<T> {
        return this.applyToComponent {
            validators.add(this.validateInput(parentDisposable, *conditions))
        }
    }

    override fun _commit(finishChosen: Boolean) = onApplyCallbacks.forEach { it.invoke() }

    //TODO: find a good icon and use here
    override fun getIcon(): Icon? = null

    override fun getComponent(): JComponent = panel

    override fun getPreferredFocusedComponent(): JComponent? = panel

    protected fun addHtml(htmlResource: String) {
        val text = this.javaClass.classLoader.getResource(htmlResource)?.readText()
            ?: error("Unable to get resource: $htmlResource")
        addComponentToStep(createHtmlComponent(text))
    }

    private fun addComponentToStep(component: JComponent) {
        // From BoxLayout docs: all components should have the same alignmentX
        component.alignmentX = Component.LEFT_ALIGNMENT
        panel.add(component)
    }

    protected fun validate(): JComponent? {
        return validators.find { !it.isValid() }?.component
    }

    open fun canProceedToNextStep(): Boolean {
        val validationResult = validate()
        validationResult?.requestFocus()
        return validationResult == null
    }

    private fun createHtmlComponent(html: String): JComponent = object : HtmlPanel() {
        init {
            update()
            adjustHeightToTextHeight(STEP_WIDTH)
        }

        override fun getBodyFont(): Font = UIUtil.getButtonFont().deriveFont(Font.PLAIN)
        override fun getBody() = html

        fun adjustHeightToTextHeight(parentWidth: Int) {
            // to use modelToView2D the size of JTextComponent must be > 0
            size = preferredSize
            val rectangleWrappingText = modelToView2D(document.length)
            preferredSize =
                Dimension(parentWidth, rectangleWrappingText.y.toInt() + rectangleWrappingText.height.toInt())
            maximumSize = preferredSize
            update()
        }
    }

    companion object {
        private const val STEP_HEIGHT = 400
        private const val STEP_WIDTH = 800
    }
}
